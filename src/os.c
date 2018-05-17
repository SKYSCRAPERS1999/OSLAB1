#include <os.h>
#include <mylib.h>

//#define MTTEST
#define SEMTEST

static void os_init();
static void os_run();
static _RegSet *os_interrupt(_Event ev, _RegSet *regs);

MOD_DEF(os) {
  .init = os_init,
  .run = os_run,
  .interrupt = os_interrupt,
};

static void os_init() {
  for (const char *p = "Hello, OS World!\n"; *p; p++) {
    _putc(*p);
  }
}

#ifdef MTTEST
extern spinlock_t lock;
static void f(void* arg) {
  while (1) {
    kmt->spin_lock(&lock);
    for (volatile int i = 0, t = uptime(); uptime() - t < 500 ; i++);
    for (volatile int i = 0; i < 20; i++) printf("%c%c", arg, "\0\n"[i==20-1]);
    kmt->spin_unlock(&lock);
  }
}
static void test_run() {
  thread_t threads[16];
  for (int i = 0; i < 16; i++) kmt->create(&threads[i], f, (void *)('a' + i));
}
#endif

#ifdef SEMTEST
static sem_t empty, fill;
static thread_t t1, t2;
static void producer() {
  while (1) {
    kmt->sem_wait(&empty);
    for (volatile int i = 0, t = uptime(); uptime() - t < 300 ; i++);
    _putc('(');
    kmt->sem_signal(&fill);
  }
}
static void consumer() {
  while (1) {
    kmt->sem_wait(&fill);
    for (volatile int i = 0, t = uptime(); uptime() - t < 300 ; i++);
    _putc(')');
    kmt->sem_signal(&empty);
  }
}
static void test_sem(){
  kmt->sem_init(&empty, "empty", SEMBUFSZ);
  kmt->sem_init(&fill, "fill", 0);
  kmt->create(&t1, producer, NULL);
  kmt->create(&t2, consumer, NULL);
}
#endif

static void os_run() {

  #ifdef MTTEST
    test_run();
  #endif

  #ifdef SEMTEST
    test_sem();
  #endif

  //printf("start\n");
  _intr_write(1); // enable interrupt
  while (1); // should never return
}

static int thread_id = -1;
extern thread_t tlist[MAXTRD];

static _RegSet *os_interrupt(_Event ev, _RegSet *regs) {

  if (thread_id != -1) memcpy(tlist[thread_id].reg, regs, REGSZ);
  thread_t *t = kmt->schedule();
  if (t != NULL) thread_id = t->id;

  #if (!defined(MTTEST) && !defined(SEMTEST))

    if (ev.event == _EVENT_IRQ_TIMER) {
        _putc('*');
    }
    if (ev.event == _EVENT_IRQ_IODEV){
        _putc('I');
    }
    if (ev.event == _EVENT_ERROR) {
        _putc('x');
      _halt(1);
    }

  #endif

  if (t != NULL) return t->reg; // this is allowed by AM
  else return NULL;
}
