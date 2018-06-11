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

file_t file_table[NINODES];

int nbitmap = 1, nimap = 1, ninodeblocks = 5;
int nmeta;    // Number of meta blocks (boot, sb, nlog, inode, bitmap)
int nblocks;  // Number of data blocks
char zeroes[BSIZE];
static void readb(struct filesystem *fs, void *dst, int bid) {  
    memcpy(dst, fs->data + bid * BSIZE, BSIZE);
}
static void writeb(struct filesystem *fs, void *src, int bid) {
    memcpy(fs->data + bid * BSIZE, src, BSIZE);
}

uint8_t mounted[3] = {0,0,0};
char mounted_name[20];
filesystem_t* FS[3];
//fsopt

fsops_t kvfs_ops;
static void fsops_init(struct filesystem *fs, const char *name, inode_t *dev){

	if (strcmp(name, "kvfs") == 0) fs->sb.type = KVFS;
	else if (strcmp(name, "procfs") == 0) fs->sb.type = PROCFS;
	else if (strcmp(name, "devfs") == 0) fs->sb.type = DEVFS;
	else { printf("Undefined fsops name"); return; }

	nmeta = 1 + ninodeblocks + nbitmap;
	nblocks = FSSIZE - nmeta;

	fs->sb.size = FSSIZE;
	fs->sb.nblocks = nblocks;
	fs->sb.ninodes = NINODES;
	fs->sb.imap_start = 0;
	fs->sb.bitmap_start = nimap;
	fs->sb.inode_start = nbitmap + nimap;
	fs->sb.datablk_start = nbitmap + nimap + ninodeblocks;

	printf("nmeta %d (inode blocks %u, bitmap blocks %u) blocks %d total %d\n", nmeta, ninodeblocks, nbitmap, nblocks, FSSIZE);
    writeb(fs, zeroes, fs->sb.imap_start);
    writeb(fs, zeroes, fs->sb.bitmap_start);

}

static void create_kvfs() {
  FS[KVFS] = (filesystem_t *)pmm->alloc(sizeof(filesystem_t));
  if (!FS[KVFS]) panic("fs allocation failed");
  FS[KVFS]->ops = &kvfs_ops; // 你为kvfs定义的fsops_t，包含函数的实现
  FS[KVFS]->ops->init(FS[KVFS], "procfs", NULL);
  vfs->mount("/kv", FS[KVFS]);
}

static inode_t *fsops_lookup(struct filesystem *fs, const char *path, int flags){
    return NULL;
}
static int fsops_close(inode_t *inode){
    return 0;
}
//vfs 

static void vfs_init(){
	FS[KVFS].ops.init = &fsops_init;
    FS[KVFS].ops.lookup = &fsops_lookup;
    FS[KVFS].ops.close = &fsops_close;
	create_kvfs();
	return;
}

static int vfs_access(const char *path, int mode){

	return 0;
}
static int vfs_mount(const char *path, filesystem_t *fs){
	mounted[fs->type] = 1;
	strcpy(mounted_name[fs->type], path);
	return 0;
}
static int vfs_unmount(const char *path){
	if (strcmp(path, mounted_name[KVFS]) == 0) mounted[KVFS] = 0;
	else if (strcmp(path, mounted_name[PROCFS]) == 0) mounted[PROCFS] = 0;
	else if (strcmp(path, mounted_name[DEVFS]) == 0) mounted[DEVFS] = 0;
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