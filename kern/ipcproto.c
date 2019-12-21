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
	ep->msg.bufsize = 1024;
	ep->msg.buf = malloc(ep->msg.bufsize);
	ep->server = current; /* self-mark */
	return 0;
}

static void CopyFrom(struct msgbuf *d, struct msgio *b, size_t iovnr)
{
	size_t size;
	char *p = d->buf, *end = d->buf + d->size;
	while (p < end && iovnr--) {
		size = b->size;
		if (p + size > end)
			size = end - p;
		memcpy(b->buf, p, size);
		p += size;
		b++;
	}
	d->size = 0;
}

static void CopyTo(struct msgbuf *d, struct msgio *b, size_t iovnr)
{
	size_t size;
	char *p = d->buf, *end = d->buf + d->bufsize;
	while (p < end && iovnr--) {
		size = b->size;
		if (p + size > end)
			size = end - p;
		memcpy(p, b->buf, size);
		p += size;
		b++;
	}
	d->size = p - d->buf;
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
	cp->rep.bufsize = 1024;
	cp->rep.buf = malloc(cp->rep.bufsize);
	cp->client = current; /* self-mark */
	return 0;
}

int sys_MsgSend(int fd, struct msgio *b, size_t iovnr,
		struct msgio *r_b, size_t r_iovnr)
{
	struct connection *cp;
	struct endpoint *ep;
	cp = current->conn + fd;
	if (!cp->client) {
		errno = EBADF; /* fd invalid */
		return -1;
	}
	assert(cp->client == current);

	cp->replied = 0;
	ep = cp->endp;
	/* add myself upon that list */
	list_insert_head(&cp->ipc_list, &ep->waiting);
	if (ep->state != EP_FREE) {
		assert(ep->waiting.first != NULL);
		while (ep->state != EP_WAIT) {	/* waiting for recver */
			sys_pause();
		}
	}
	CopyTo(&ep->msg, b, iovnr);

	if (ep->state == EP_WAIT)
		ep->server->state = 0;
	ep->state = EP_SENT;

	while (!cp->replied) {	/* waiting for server reply */
		sys_pause();
	}
	CopyFrom(&cp->rep, r_b, r_iovnr);
	cp->replied = 0;
	return 0;
}

int sys_MsgReceive(id_t chid, struct msgio *b, int iovnr)
{
	struct list_node *head;
	struct endpoint *ep;
	ep = current->channels + chid;
	if (!ep->server) {
		errno = ENOENT; /* chid invalid */
		return -1;
	}
	assert(ep->server == current);

	if (ep->state == EP_FREE) {
		assert(ep->waiting.first == NULL);
		ep->state = EP_WAIT;
	}
	while (ep->state == EP_WAIT) {	/* waiting for sender */
		sys_pause();
	}
	if (ep->state == EP_SENT) {	/* sender ready */
		CopyFrom(&ep->msg, b, iovnr);
		head = ep->waiting.first;
		assert(head != NULL);
		ep->reply = list_entry(head, struct connection, ipc_list);
		list_remove(head);
		ep->state = EP_RCVD;

	} else {
		assert(ep->state == EP_RCVD);
		/* another thread running this channel */
		errno = EBUSY;
		return -1;
	}
	return 0;
}

int sys_MsgReply(id_t chid, struct msgio *b, int iovnr)
{
	struct connection *cp;
	struct endpoint *ep;
	ep = current->channels + chid;
	if (!ep->server) {
		errno = ENOENT; /* chid invalid */
		return -1;
	}
	assert(ep->server == current);

	if (ep->state != EP_RCVD) {
		/* cannot reply: you haven't recved any yet */
		errno = EBUSY;
		return -1;
	}
	assert(ep->reply != NULL);

	cp = ep->reply;
	cp->replied = 1;
	cp->client->state = 0;
	CopyTo(&cp->rep, b, iovnr);
	ep->state = ep->waiting.first ? EP_SENT : EP_FREE;

	return 0;
}
