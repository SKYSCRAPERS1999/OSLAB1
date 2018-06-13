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

uint8_t mounted[3] = {0,0,0};
char mounted_name[3][20];
filesystem_t* FS[3];

file_t* ftable[MAXFILENUM];
//fsopt

fsops_t kvfs_ops, procfs_ops, devfs_ops;

static void fsops_init(struct filesystem *fs, const char *name, inode_t *dev){
	if (strcmp(name, "kvfs") == 0) fs->type = KVFS;
	else if (strcmp(name, "procfs") == 0) fs->type = PROCFS;
	else if (strcmp(name, "devfs") == 0) fs->type = DEVFS;
	else { panic("Undefined fsops name"); return; }
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
  FS[DEVFS]->ops = &devfs_ops; // 你为devfs定义的fsops_t，包含函数的实现
  FS[DEVFS]->ops->init(FS[DEVFS], "devfs", NULL);
  vfs->mount("/", FS[DEVFS]);
}

//file
static int find_file(int fd){
  for (int i = 0; i < NFILE; i++){
    if (ftable[i] != NULL && ftable[i]->fd == fd) return i;
  }
  return -1;
}

static int fileops_open(inode_t *inode, file_t *file, int mode){
	if (inode == NULL || (inode->type == O_RDONLY && mode == O_WRONLY) || (inode->type == O_WRONLY && mode == O_RDONLY)){
		panic("Invalid mode or inode!");
		return -1;
	}
	file->mode = mode;
	file->off = 0;
	for (int i = 0; i < NDIRECT; i++){
		if (inode->file[i] != NULL && inode->file[i]->fd == file->fd){
			return file->fd;	
		} 
	}
	for (int i = 0; i < NDIRECT; i++){
		if (inode->file[i] == NULL) {
			inode->file[i] = file;
			file->inode = inode;
			return file->fd;
		}
	}
	panic("Maybe insufficient inode space");
	return -1;
}
static ssize_t fileops_read(inode_t *inode, file_t *file, char *buf, size_t size){
	if (inode == NULL || file->mode == O_WRONLY) {
		panic("Invalid mode or inode!");
	}
	int len = (file->off + size > strlen(inode->data) ? strlen(inode->data) - file->off : size);
	if (len < 0) len = 0;
	memcpy(buf, inode->data + file->off, len);
	buf[size] = '\0';
	file->off += len;
	return len;
}
static ssize_t fileops_write(inode_t *inode, file_t *file, const char *buf, size_t size){
	if (inode == NULL || file->mode == O_WRONLY) {
		panic("Invalid mode or inode!");
	}
	int len = (file->off + size > strlen(inode->data) ? strlen(inode->data) - file->off : size);
	if (len < 0) len = 0;
	memcpy(inode->data + file->off, buf, len);
	inode->data[file->off + len] = '\0';
	file->off += len;
	return len;
}

static off_t fileops_lseek(inode_t *inode, file_t *file, off_t offset, int whence){
	return 0;
}

static int fileops_close(inode_t *inode, file_t *file, int p){
	for (int i = 0; i < NDIRECT; i++){
		if (inode->file[i] != NULL && inode->file[i]->fd == file->fd){
			inode->file[i] = NULL
			break;
		}
	}
	pmm->free(file);
	ftable[p] = NULL;
	return 0;
}
//vfs 

static void vfs_init(){
	for (int i = 0; i < NFILE; i++){
    	ftable[i] = NULL;
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
		handle = FS[KVFS]->ops->lookup(FS[KVFS], lpath + strlen("/"), mode);
	}else if ((lpath = strstr(path, "/procfs")) != NULL){
		handle = FS[PROCFS]->ops->lookup(FS[PROCFS], lpath + strlen("/procfs"), mode);
	}else if ((lpath = strstr(path, "/devfs")) != NULL){
		handle = FS[DEVFS]->ops->lookup(FS[DEVFS], lpath + strlen("/devfs"), mode);
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

static int vfs_open(const char *path, int mode/*flags?*/){
	char* lpath = NULL;
	inode_t* handle = NULL;
	if ((lpath = strstr(path, "/")) != NULL){
		handle = FS[KVFS]->ops->lookup(FS[KVFS], lpath + strlen("/"), mode);
	}else if ((lpath = strstr(path, "/procfs")) != NULL){
		handle = FS[PROCFS]->ops->lookup(FS[PROCFS], lpath + strlen("/procfs"), mode);
	}else if ((lpath = strstr(path, "/devfs")) != NULL){
		handle = FS[DEVFS]->ops->lookup(FS[DEVFS], lpath + strlen("/devfs"), mode);
	}else{
		panic("Invalid access!");
		return -1;
	}
	
	if (handle == NULL || (handle->type == O_RDONLY && mode == O_WRONLY) || (handle->type == O_WRONLY && mode == O_RDONLY)){
		panic("Invalid mode!");
		return -1;
	}

	file_t* f = (file_t *)pmm->alloc(sizeof(file_t));
	if (f == NULL) {
		panic("Allocation failed"); return -1;
	}
	int fd = fileops_open(handle, f, mode); 
	int cnt;
	for (cnt = 0; cnt < NFILE; cnt++){
		if (ftable[cnt] == NULL || ftable[cnt]->ref = 0) {
			ftable[cnt] = f;
			ftable[cnt]->ref = 1;
			break;
		}
	}
	if (cnt >= NFILE){
		panic("Allocation failed"); return -1;
	}
	return fd;
}
static ssize_t vfs_read(int fd, void *buf, size_t nbyte){
	int p = find_file(fd);
    if (p == -1) return -1;
    return fileops_read(ftable[p]->inode, ftable[p], buf, nbyte);
}
static ssize_t vfs_write(int fd, void *buf, size_t nbyte){
	int p = find_file(fd);
    if (p == -1) return -1;
    return fileops_write(ftable[p]->inode, ftable[p], buf, nbyte);
}
static off_t vfs_lseek(int fd, off_t offset, int whence){
	int p = find_file(fd);
    if (p == -1) return -1;
    return fileops_lseek(ftable[p]->inode, ftable[p], offset, whence);
}

static int vfs_close(int fd){
	int p = find_file(fd);
    if (p == -1) return -1;
    return fileops_close(ftable[p]->inode, ftable[p], p);
	return 0;
}