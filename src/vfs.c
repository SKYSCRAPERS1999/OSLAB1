#include <os.h>
#include <mylib.h>
#include <fs.h>

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

int fsfd;
struct superblock sb;
char zeroes[BSIZE];
uint freeinode = 1, freeblock;
fsop_t kvfs_ops, procfs_ops, devfs_ops;

static filesystem_t *create_kvfs() {
  filesystem_t *fs = (filesystem_t *)pmm->alloc(sizeof(filesystem_t));
  if (!fs) panic("fs allocation failed");
  fs->ops = &kvfs_ops; // 你为kvfs定义的fsops_t，包含函数的实现
  fs->ops->init(fs, "procfs", NULL);
  return fs;
}

static void vfs_init(){
	create_kvfs();
	return;
}
static int vfs_access(const char *path, int mode){

	return 0;
}
static int vfs_mount(const char *path, filesystem_t *fs){

	return 0;
}
static int vfs_unmount(const char *path){

	return 0;
}
static int vfs_open(const char *path, int flags){

	return 0;
}
static ssize_t vfs_read(int fd, void *buf, size_t nbyte){

	return 0;
}
static ssize_t vfs_write(int fd, void *buf, size_t nbyte){

	return 0;
}
static off_t vfs_lseek(int fd, off_t offset, int whence){

	return 0;
}
static int vfs_close(int fd){

	return 0;
}