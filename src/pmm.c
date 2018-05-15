#include <os.h>
#include <mylib.h>

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

static void* init(){
	dict_cnt = 1;
	dict[0] = {_heap.start, _heap.end - heap.start, 1};
}

static void* alloc(size_t size){
	int align = 1;
	while (align < size) align <<= 1;
	for (int i = 0; i < dict_cnt; i++){
		if (dict[i].freed && dict[i].addr >= size){
				
		}
	}
}

static void* free(void* ptr){


}
