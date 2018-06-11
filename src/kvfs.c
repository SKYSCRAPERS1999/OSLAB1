/*
#include <os.h>
#include <fs.h>
#include <mylib.h>
// [ boot block | sb block | log | inode blocks | free bit map | data blocks ]

static void kvfs_init(struct filesystem *fs, const char *name, inode_t *dev);
static inode_t *kvfs_lookup(struct filesystem *fs, const char *path, int flags);
static int kvfs_close(inode_t *inode);

MOD_DEF(kvfs){
	.init = kvfs_init,
	.lookup = kvfs_access,
	.close = kvfs_close,
};

static int kvfile_open(inode_t *inode, file_t *file, int flags);
static ssize_t kvfile_read(inode_t *inode, file_t *file, char *buf, size_t size);
static ssize_t kvfile_write(inode_t *inode, file_t *file, const char *buf, size_t size);
static off_t kvfile_lseek(inode_t *inode, file_t *file, off_t offset, int whence);

MOD_DEF(kvfile){
	.open = kvfile_open,
	.read = kvfile_read,
	.write = kvfile_write,
	.lseek = kvfile_lseek,
};

static struct file* filealloc(){
  struct file *f;
  for (f = ftable.file; f < ftable.file + NFILE; f++){
    if (f->ref == 0){
 	  f->ref = 1;
      return f;
    }
  }
  return 0;
}




void readb(struct filesystem *fs, void *dst, int bid) {  
    memcpy(dst, fs->data + bid * BSIZE, BSIZE);
}
void writeb(struct filesystem *fs, void *src, int bid) {
    memcpy(fs->data + bid * BSIZE, src, BSIZE);
}

static void kvfs_init(struct filesystem *fs, const char *name, inode_t *dev){
	

}


static inode_t *kvfs_lookup(struct filesystem *fs, const char *path, int flags){

}

static int kvfs_close(inode_t *inode){


}

*/