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
    int freed;
} mem_dict;
mem_dict dict[100000];
static int dict_cnt;

static void pmm_init(){
	dict_cnt = 1;
	dict[0] = (mem_dict){ _heap.start, _heap.end - _heap.start, 1};
}

static void* pmm_alloc(size_t size){
	int align = 1;
	while (align < size) align <<= 1;
	for (int i = 0; i < dict_cnt; i++){
		if (dict[i].freed && (int)dict[i].addr >= size){
				
		}
	}
}

static void pmm_free(void* ptr){


}
