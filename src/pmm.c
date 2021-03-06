#include <os.h>
#include <mylib.h>

static void pmm_init();
static void* pmm_alloc(size_t);
static void pmm_free(void*);

MOD_DEF(pmm) {
	.init = pmm_init,
	.alloc = pmm_alloc,
	.free = pmm_free,
};

typedef struct node{
    void* addr;
    size_t size;
} node;
node free_dict[300000];
node used_dict[300000];

static int free_cnt, used_cnt;

static void pmm_init(){
	memset(free_dict, 0, sizeof(free_dict));
	memset(used_dict, 0, sizeof(used_dict));
	free_cnt = 1, used_cnt = 0;
	free_dict[0] = (node){ _heap.start, _heap.end - _heap.start};
}

static void* pmm_alloc(size_t size){
	int bitcnt = 1, align;
	for (int i = 1; i <= size; i <<= 1) bitcnt++;
	align = 0x7fffffff ^ ((1 << bitcnt) - 1);
	for (int i = 0; i < free_cnt; i++){
		int align_addr = ((int)free_dict[i].addr & align);
		if (align_addr < (int)free_dict[i].addr) align_addr += (1 << bitcnt);

		int dict_lim = (int)free_dict[i].addr + free_dict[i].size;
		int dict_r = align_addr + size;
		if (dict_lim >= dict_r){
			used_dict[used_cnt++] = (node){(void*)dict_r, dict_lim - dict_r};

			void* ret = (void*)align_addr;
			free_dict[i].addr = (void*)dict_r;
			free_dict[i].size = dict_lim - dict_r;
			return ret;
		}
	}
	return NULL;
}

static void pmm_free(void* ptr){
	for (int i = 0; i < used_cnt; i++){
		if (used_dict[i].addr == ptr){
			free_dict[free_cnt++] = (node){used_dict[i].addr, used_dict[i].size};
			used_dict[i] = used_dict[used_cnt];
			--used_cnt;
			return;
		}
	}
}
