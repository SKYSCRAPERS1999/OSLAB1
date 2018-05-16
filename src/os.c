#include <os.h>
#include <mylib.h>

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

static void f(void *arg) {
  while (1) {
    _putc((char)arg);
  }
}

static void test_run() {
  thread_t t1;
  kmt->create(&t1, f, (void *)'a');
}

static void os_run() {

  printf("Hello, OS World!\n");
  for (volatile int sz = 1e5; sz > 0; sz-=1e3){
    void *ptr = pmm->alloc(sz);
    printf("0x%x\n", (int)ptr);
    if (!ptr) {
      panic("memory allocation failed");
    }
    pmm->free(ptr);
  }
  
  _intr_write(1); // enable interrupt
  while (1) ; // should never return
}

static _RegSet *os_interrupt(_Event ev, _RegSet *regs) {
  if (ev.event == _EVENT_IRQ_TIMER) _putc('*');
  if (ev.event == _EVENT_IRQ_IODEV) _putc('I');
  if (ev.event == _EVENT_ERROR) {
    _putc('x');
    _halt(1);
  }
  return NULL; // this is allowed by AM
}
