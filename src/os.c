#include <os.h>
#include <mylib.h>

#define TESTRUN

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

static void f(void* arg) {
  while (1) {
    printf("%c", arg);
  }
}

static void test_run() {
  thread_t t1, t2;
  kmt->create(&t1, f, (void *)'a');
  kmt->create(&t2, f, (void *)'b');
  _yield();
}

static void os_run() {

  #ifdef TESTRUN
    test_run();
  #endif  
    printf("return\n");
  _intr_write(1); // enable interrupt
  while (1) ; // should never return
}

static int thread_id = -1;
extern thread_t tlist[MAXTRD];

static _RegSet *os_interrupt(_Event ev, _RegSet *regs) {
  if (thread_id != -1) {
      memcpy(tlist[thread_id].reg, regs, REGSZ);
  }
  thread_t *t = kmt->schedule();
  thread_id = t->id;

  if (ev.event == _EVENT_IRQ_TIMER) ;//_putc('*');
  if (ev.event == _EVENT_IRQ_IODEV) ;//_putc('I');
  if (ev.event == _EVENT_ERROR) {
    //_putc('x');
    _halt(1);
  }
  return NULL; // this is allowed by AM
}
