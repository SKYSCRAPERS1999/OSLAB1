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
struct {
  // struct spinlock lock;
  struct file file[NFILE];
} ftable;
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

int nbitmap = FSSIZE/(BSIZE*8) + 1;
int ninodeblocks = NINODES / IPB + 1;
int nmeta;    // Number of meta blocks (boot, sb, nlog, inode, bitmap)
int nblocks;  // Number of data blocks
char zeroes[BSIZE];

static void kvfs_init(struct filesystem *fs, const char *name, inode_t *dev){
	
	nmeta = 1 + ninodeblocks + nbitmap;
	nblocks = FSSIZE - nmeta;

	fs->sb.size = FSSIZE;
	fs->sb.nblocks = nblocks;
	fs->sb.ninodes = NINODES;
	fs->sb.inodestart = 1;
	fs->sb.bmapstart = 1 + ninodeblocks;
	printf("nmeta %d (inode blocks %u, bitmap blocks %u) blocks %d total %d\n", nmeta, ninodeblocks, nbitmap, nblocks, FSSIZE);
	memset(fs->ibitmap, 0, BSIZE);
    memset(fs->bbitmap, 0, BSIZE);
    
    

}

static inode_t *kvfs_lookup(struct filesystem *fs, const char *path, int flags){

}
static int kvfs_close(inode_t *inode){


}