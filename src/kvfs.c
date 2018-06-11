#include <os.h>
#include <mylib.h>

static void init(struct filesystem *fs, const char *name, inode_t *dev);
static inode_t *lookup(struct filesystem *fs, const char *path, int flags);
static int close(inode_t *inode);

MOD_DEF(kvfs){
	.init = kvfs_init,
	.lookup = kvfs_access,
	.close = kvfs_close,
};

static void init(struct filesystem *fs, const char *name, inode_t *dev){
	
}

static inode_t *lookup(struct filesystem *fs, const char *path, int flags){

}
static int close(inode_t *inode){


}