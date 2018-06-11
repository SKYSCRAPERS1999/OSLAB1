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
/*static void readb(struct filesystem *fs, void *dst, int bid) {  
    memcpy(dst, fs->blocks + bid * BSIZE, BSIZE);
}*/
static void writeb(struct filesystem *fs, void *src, int bid) {
    memcpy(fs->blocks + bid * BSIZE, src, BSIZE);
}

uint8_t mounted[3] = {0,0,0};
char mounted_name[3][20];
filesystem_t* FS[3];
file_t ftable[NFILE];

//fsopt

fsops_t kvfs_ops, procfs_ops, devfs_ops;

static void fsops_init(struct filesystem *fs, const char *name, inode_t *dev){

	if (strcmp(name, "kvfs") == 0) fs->sb.type = KVFS;
	else if (strcmp(name, "procfs") == 0) fs->sb.type = PROCFS;
	else if (strcmp(name, "devfs") == 0) fs->sb.type = DEVFS;
	else { panic("Undefined fsops name"); return; }

	nmeta = 1 + ninodeblocks + nbitmap;
	nblocks = FSSIZE - nmeta;

	fs->sb.size = FSSIZE;
	fs->sb.nblocks = nblocks;
	fs->sb.ninodes = NINODES;
	fs->sb.imap_start = 0;
	fs->sb.bitmap_start = nimap;
	fs->sb.inode_start = nbitmap + nimap;
	fs->sb.datablk_start = nbitmap + nimap + ninodeblocks;

	printf("nmeta %d (inode blocks %d, bitmap blocks %d) blocks %d total %d\n", nmeta, ninodeblocks, nbitmap + nimap, nblocks, FSSIZE);
    writeb(fs, zeroes, fs->sb.imap_start);
    writeb(fs, zeroes, fs->sb.bitmap_start);
}

static inode_t *fsops_lookup(struct filesystem *fs, const char *path, int flags){

    return NULL;
}
static int fsops_close(inode_t *inode){
    
    return 0;
}

static void create_kvfs() {
  FS[KVFS] = (filesystem_t *)pmm->alloc(sizeof(filesystem_t));
  if (NULL == FS[KVFS]) panic("fs allocation failed");
  FS[KVFS]->ops = &kvfs_ops; // 你为kvfs定义的fsops_t，包含函数的实现
  FS[KVFS]->ops->init(FS[KVFS], "kvfs", NULL);
  vfs->mount("/", FS[KVFS]);
}
static void create_procfs() {
  FS[PROCFS] = (filesystem_t *)pmm->alloc(sizeof(filesystem_t));
  if (NULL == FS[PROCFS]) panic("fs allocation failed");
  FS[PROCFS]->ops = &procfs_ops; // 你为procfs定义的fsops_t，包含函数的实现
  FS[PROCFS]->ops->init(FS[PROCFS], "procfs", NULL);
  vfs->mount("/", FS[PROCFS]);
}
static void create_devfs() {
  FS[DEVFS] = (filesystem_t *)pmm->alloc(sizeof(filesystem_t));
  if (NULL == FS[DEVFS]) panic("fs allocation failed");
  FS[DEVFS]->ops = &devfs_ops; // 你为kvfs定义的fsops_t，包含函数的实现
  FS[DEVFS]->ops->init(FS[DEVFS], "devfs", NULL);
  vfs->mount("/", FS[DEVFS]);
}

//file
fileops_t file_ops;

static struct file* filealloc(){
  file_t *f;
  for (f = ftable; f < ftable + NFILE; f++){
    if (f->ref == 0){
 	  f->ref = 1;
      return f;
    }
  }
  return 0;
}
static int fileops_open(inode_t *inode, file_t *file, int flags){
	return 0;
}
static ssize_t fileops_read(inode_t *inode, file_t *file, char *buf, size_t size){
	return 0;
}
static ssize_t fileops_write(inode_t *inode, file_t *file, const char *buf, size_t size){
	return 0;
}
static off_t fileops_lseek(inode_t *inode, file_t *file, off_t offset, int whence){
	return 0;
}

//vfs 

static void vfs_init(){
	int cnt = 0;
	for (file_t* f = ftable; f < ftable + NFILE; f++) {
		f->ref = f->readable = f->writable = f->off = 0;
		f->id = cnt++; f->ip = NULL;
	}
	kvfs_ops.init = &fsops_init; kvfs_ops.lookup = &fsops_lookup; kvfs_ops.close = &fsops_close;
    procfs_ops.init = &fsops_init; procfs_ops.lookup = &fsops_lookup; procfs_ops.close = &fsops_close;
    devfs_ops.init = &fsops_init; devfs_ops.lookup = &fsops_lookup; devfs_ops.close = &fsops_close;
	create_kvfs(); create_procfs(); create_devfs();
	return;
}

static int vfs_access(const char *path, int mode){

	char* lpath = NULL;
	inode_t* handle = NULL;
	if ((lpath = strstr(path, "/")) != NULL){
		handle = FS[KVFS]->ops->lookup(FS[KVFS], lpath, mode);
	}else if ((lpath = strstr(path, "/procfs")) != NULL){
		handle = FS[PROCFS]->ops->lookup(FS[PROCFS], lpath, mode);
	}else if ((lpath = strstr(path, "/devfs")) != NULL){
		handle = FS[DEVFS]->ops->lookup(FS[DEVFS], lpath, mode);
	}else{
		panic("Invalid access!");
		return -1;
	}

	if (handle == NULL || (handle->type == O_RDONLY && mode == O_WRONLY) || (handle->type == O_WRONLY && mode == O_RDONLY)){
		panic("Invalid mode!");
		return -1;
	}

	return 0;
}
static int vfs_mount(const char *path, filesystem_t *fs){
	mounted[fs->sb.type] = 1;
	strcpy(mounted_name[fs->sb.type], path);
	return 0;
}
static int vfs_unmount(const char *path){
	if (strcmp(path, mounted_name[KVFS]) == 0) mounted[KVFS] = 0;
	else if (strcmp(path, mounted_name[PROCFS]) == 0) mounted[PROCFS] = 0;
	else if (strcmp(path, mounted_name[DEVFS]) == 0) mounted[DEVFS] = 0;
	return 0;
}

static int vfs_open(const char *path, int mode/*flags?*/){
	char* lpath = NULL;
	inode_t* handle = NULL;
	if ((lpath = strstr(path, "/")) != NULL){
		handle = FS[KVFS]->ops->lookup(FS[KVFS], lpath, mode);
	}else if ((lpath = strstr(path, "/procfs")) != NULL){
		handle = FS[PROCFS]->ops->lookup(FS[PROCFS], lpath, mode);
	}else if ((lpath = strstr(path, "/devfs")) != NULL){
		handle = FS[DEVFS]->ops->lookup(FS[DEVFS], lpath, mode);
	}else{
		panic("Invalid access!");
		return -1;
	}
	
	if (handle == NULL || (handle->type == O_RDONLY && mode == O_WRONLY) || (handle->type == O_WRONLY && mode == O_RDONLY)){
		panic("Invalid mode!");
		return -1;
	}



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