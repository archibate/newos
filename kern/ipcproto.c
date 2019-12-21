#include <kern/sched.h>
#include <kern/kernel.h>
#include <bits/lepton.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>

int sys_CreateChannel(id_t chid)
{
	struct endpoint *ep;
	ep = current->channels + chid;
	if (ep->server) {
		assert(ep->server == current);
		errno = EEXIST; /* chid already in use */
		return -1;
	}
	ep->bufsize = 1024;
	ep->buf = malloc(ep->bufsize);
	ep->server = current; /* self-mark */
	return 0;
}

static void CopyFrom(struct endpoint *ep, struct msgio *b, size_t iovnr)
{
	size_t size;
	char *p = ep->buf, *end = ep->buf + ep->size;
	assert(ep->state == EP_SENT);
	while (p < end && iovnr--) {
		size = b->size;
		if (p + size > end)
			size = end - p;
		memcpy(b->buf, p, size);
		p += size;
		b++;
	}
	ep->size = 0;
}

static void CopyTo(struct endpoint *ep, struct msgio *b, size_t iovnr)
{
	size_t size;
	char *p = ep->buf, *end = ep->buf + ep->bufsize;
	assert(ep->state == EP_WAIT || ep->state == EP_FREE);
	while (p < end && iovnr--) {
		size = b->size;
		if (p + size > end)
			size = end - p;
		memcpy(p, b->buf, size);
		p += size;
		b++;
	}
	ep->size = p - ep->buf;
}

static struct endpoint *GetEndpoint(pid_t pid, id_t chid)
{
	struct task *server;
	struct endpoint *ep;
	int i = get_pid_index(pid);
	if (i == -1) {
		errno = ESRCH; /* pid invalid */
		return NULL;
	}
	server = task[i];

	ep = server->channels + chid;
	if (!ep->server) {
		errno = ENOENT; /* chid invalid */
		return NULL;
	}
	assert(ep->server == server);
	return ep;
}

int sys_Connect(int fd, pid_t pid, id_t chid)
{
	struct connection *cp;
	struct endpoint *ep;
	cp = current->conn + fd;
	if (cp->client) {
		assert(cp->client == current);
		errno = EEXIST; /* fd already in use */
		return -1;
	}

	ep = GetEndpoint(pid, chid);
	if (!ep)
		return -1;
	cp->endp = ep;
	cp->client = current; /* self-mark */
	return 0;
}

int sys_MsgSend(int fd, struct msgio *b, size_t iovnr)
{
	struct connection *cp;
	struct endpoint *ep;
	cp = current->conn + fd;
	if (!cp->client) {
		errno = EBADF; /* fd invalid */
		return -1;
	}
	assert(cp->client == current);

	ep = cp->endp;
	/* add myself upon that list */
	list_insert_head(&current->ipc_list, &ep->sender);
	if (ep->state != EP_FREE) {
		while (ep->state != EP_WAIT) {	/* waiting for recver */
			sys_pause();
		}
	}
	CopyTo(ep, b, iovnr);
	ep->state = EP_SENT;
	// TODO: waiting for reply
	return 0;
}

int sys_MsgReceive(id_t chid, struct msgio *b, int iovnr)
{
	struct endpoint *ep;
	ep = current->channels + chid;
	if (!ep->server) {
		errno = ENOENT; /* chid invalid */
		return -1;
	}
	assert(ep->server == current);

	if (ep->state == EP_FREE) {
		ep->state = EP_WAIT;
	}
	while (ep->state == EP_WAIT) {	/* waiting for sender */
		sys_pause();
	}
	if (ep->state == EP_SENT) {	/* sender ready */
		CopyFrom(ep, b, iovnr);
		ep->state = EP_RCVD;

	} else {
		assert(ep->state == EP_RCVD);
		/* another thread running this channel */
		errno = EBUSY;
		return -1;
	}
	return 0;
}
