#include <os.h>
#include <mylib.h>

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
	srand((int)uptime());
}

static int kmt_create(thread_t *thread, void (*entry)(void *arg), void *arg){
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

	return thread->id;
}

static void kmt_teardown(thread_t *thread){
	pmm->free(thread->kstack);
	thread->freed = 1;
	if (thread->id == cur_id) cur_id = -1;
}

static thread_t* kmt_schedule(){
	if (!thread_num) return NULL;

	int thread_idx = -1, chg = 0;
	//if (cur_id != -1) chg = 1;
	for (int i = 0, j = rand()%thread_num; i < thread_num; i++, j = (j+1)%thread_num) {
		if (chg && j == cur_id) continue;
		if (!tlist[j].freed) {
			cur_id = thread_idx = j; 
			break;
		}
	}
	_debug("thread_num,thread_idx,cur_id=%d,%d,%d\n",thread_num, thread_idx,cur_id);
	if (thread_idx != -1) return &tlist[thread_idx];
	else if (cur_id != -1) return &tlist[cur_id];
	else return NULL;
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
	sem->id = -1;
	sem->name = name;
	sem->count = value;
}

static void kmt_sem_wait(sem_t *sem){
	--sem->count;
	if (sem->count < 0){
		if (cur_id == -1) {
			panic("No Current Process");
			return;
		}else{
			sem->id = cur_id;
			tlist[sem->id].freed = 1;
			_yield();
		}
	}
}

static void kmt_sem_signal(sem_t *sem){
	++sem->count;
	if (sem->id != -1){
		tlist[sem->id].freed = 0;
		sem->id = -1;
		_yield();
	}
}