#include <os.h>
#include <mylib.h>

static void vfs_init();
static int vfs_access(const char *, int);
static int vfs_mount(const char *, filesystem_t *);
static int vfs_unmount(const char *path);
static int vfs_open(const char *, int);
static ssize_t vfs_read(int, void *, size_t);
static ssize_t vfs_write(int, void *, size_t);
static off_t vfs_lseek(int , off_t offset, int);
static int vfs_close(int);

MOD_DEF(vfs){
	.init = vfs_init,
	.access = vfs_access,
	.mount = vfs_mount,
	.unmount = vfs_unmount,
	.open = vfs_open,
	.read = vfs_read,
	.write = vfs_write,
	.lseek = vfs_lseek,
	.close = vfs_close,
};

void vfs_init(){

	return;
}
int vfs_access(const char *path, int mode){

	return 0;
}
int vfs_mount(const char *path, filesystem_t *fs){

	return 0;
}
int vfs_unmount(const char *path){

	return 0;
}
int vfs_open(const char *path, int flags){

	return 0;
}
ssize_t vfs_read(int fd, void *buf, size_t nbyte){

	return 0;
}
ssize_t vfs_write(int fd, void *buf, size_t nbyte){

	return 0;
}
off_t vfs_lseek(int fd, off_t offset, int whence){

	return 0;
}
int vfs_close(int fd){

	return 0;
}