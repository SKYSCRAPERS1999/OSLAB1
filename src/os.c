#include <os.h>
#include <mylib.h>

//#define OSTEST

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

extern spinlock_t lock;

#ifdef OSTEST
static void f(void* arg) {
  while (1) {
    kmt->spin_lock(&lock);
    for (volatile int i = 0, t = uptime(); uptime() - t < 500 ; i++);
    for (volatile int i = 0; i < 20; i++) printf("%c%c", arg, "\0\n"[i==20-1]);
    kmt->spin_unlock(&lock);
  }
}
static void test_run() {
  thread_t t[16];
  for (int i = 0; i < 16; i++) kmt->create(&t[i], f, (void *)('a' + i));
}
#endif

static void os_run() {

  #ifdef OSTEST
    test_run();
  #endif

  printf("return\n");
  _intr_write(1); // enable interrupt
  while (1); // should never return
}

static int thread_id = -1;
extern thread_t tlist[MAXTRD];

static _RegSet *os_interrupt(_Event ev, _RegSet *regs) {

  if (thread_id != -1) memcpy(tlist[thread_id].reg, regs, REGSZ);
  thread_t *t = kmt->schedule();
  thread_id = t->id;

  if (ev.event == _EVENT_IRQ_TIMER) {
    #ifndef OSTEST
      _putc('*');
    #endif
  }
  if (ev.event == _EVENT_IRQ_IODEV){
    #ifndef OSTEST
      _putc('I');
    #endif
  }
  if (ev.event == _EVENT_ERROR) {
    #ifndef OSTEST
      _putc('x');
    #endif
    _halt(1);
  }
  return t->reg; // this is allowed by AM
}
