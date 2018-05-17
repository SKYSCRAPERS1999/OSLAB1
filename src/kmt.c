#include <os.h>
#include <mylib.h>

#define __LOCAL_DEBUG__

#ifdef __LOCAL_DEBUG__
# define _debug(fmt, ...) printf(stderr, "\033[94m%s: " fmt "\n\033[0m", \
    __func__, ##__VA_ARGS__)
#else
# define _debug(...) ((void) 0)
#endif

static void kmt_init();
static int kmt_create(thread_t *thread, void (*entry)(void *arg), void *arg);
static void kmt_teardown(thread_t *thread);
static thread_t* kmt_schedule();
static void kmt_spin_init(spinlock_t *lk, const char *name);
static void kmt_spin_lock(spinlock_t *lk);
static void kmt_spin_unlock(spinlock_t *lk);
static void kmt_sem_init(sem_t *sem, const char *name, int value);
static void kmt_sem_wait(sem_t *sem);
static void kmt_sem_signal(sem_t *sem);

MOD_DEF(kmt) {
	.init = kmt_init,
	.create = kmt_create,
	.teardown = kmt_teardown,
	.schedule = kmt_schedule,
	.spin_init = kmt_spin_init,
	.spin_lock = kmt_spin_lock,
	.spin_unlock = kmt_spin_unlock,
	.sem_init = kmt_sem_init,
	.sem_wait = kmt_sem_wait,
	.sem_signal = kmt_sem_signal,
};

static int cur_id = -1, thread_num = 0;

spinlock_t lock;
thread_t tlist[MAXTRD];

void kmt_init(){
	kmt->spin_init(&lock, "mutex");
}

static int kmt_create(thread_t *thread, void (*entry)(void *arg), void *arg){

	//kmt->spin_lock(&lock);
	int thread_idx = -1;
	for (int i = 0; i < thread_num; i++) {
		if (tlist[i].freed) {
			thread_idx = i; 
			break;
		}
	}
	if (thread_idx == -1) thread_idx = thread_num++;

	if (thread_num >= MAXTRD) {
		panic("Thread Number Error!");
		_halt(1);
	}

	tlist[thread_idx].freed = 0;
	tlist[thread_idx].id = cur_id = thread_idx;
	tlist[thread_idx].kstack = pmm->alloc(STKSZ);
	_Area Tkstack = {tlist[thread_idx].kstack, tlist[thread_idx].kstack+STKSZ};
	tlist[thread_idx].reg = _make(Tkstack, entry, arg);

	thread = &tlist[thread_idx];
	//kmt->spin_unlock(&lock);

	return thread->id;
}

static void kmt_teardown(thread_t *thread){
	//kmt->spin_lock(&lock);
	pmm->free(thread->kstack);
	thread->freed = 1;
	if (thread->id == cur_id) cur_id = -1;
	//kmt->spin_unlock(&lock);
}

static thread_t* kmt_schedule(){
	
	int thread_idx = -1, chg = 0;
	if (cur_id != -1) chg = 1;
	//kmt->spin_lock(&lock);
	for (int i = 0; i < thread_num; i++) {
		if (chg && i == cur_id) continue;
		if (!tlist[i].freed) {
			cur_id = thread_idx = i; 
			break;
		}
	}
	//kmt->spin_unlock(&lock);
	_debug("thread_num,thread_idx,cur_id=%d,%d,%d\n",thread_num, thread_idx,cur_id);
	if (thread_idx != -1) return &tlist[thread_idx];
	else if (cur_id != -1) return &tlist[cur_id];
	return NULL;
}

static void kmt_spin_init(spinlock_t *lk, const char *name){
	lk->name = name;
 	lk->locked = 0;
}

static void kmt_spin_lock(spinlock_t *lk){
	 _intr_write(0); // disable interrupt
	 if (lk->locked) panic("acquire");
	 while (_atomic_xchg(&lk->locked, 1) != 0);
}

static void kmt_spin_unlock(spinlock_t *lk){
	_atomic_xchg(&lk->locked, 0);
	_intr_write(1); // enable interrupt
}

static void kmt_sem_init(sem_t *sem, const char *name, int value){

}

static void kmt_sem_wait(sem_t *sem){

}

static void kmt_sem_signal(sem_t *sem){

}