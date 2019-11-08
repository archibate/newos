#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ds/unitest.h>

int main(void)
{
	TBLK {
		T(mkdir("a", 0644), != -1);
		T(creat("a/b", 0644 | S_IFREG), == -1 && errno == EACCES);
		T(rmdir("a"), != -1);
		T(mkdir("a", 0755), != -1);
		T(creat("a/b", 0644 | S_IFREG), != -1);
		T(rmdir("a"), == -1 && errno == ENOTEMPTY);
		T(unlink("a/b/"), == -1 && errno == ENOTDIR);
		T(unlink("a/b"), != -1);
		T(chdir("a"), != -1);
		T(rmdir("../a"), != -1);
		T(open(".", O_DIRECTORY), == -1 && errno == ENOENT);
	} ENDT;
}
