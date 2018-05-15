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

static void os_run() {
  printf("Hello, OS World!\n");
  for (long long sz = 1; sz < 1e5; sz++){
    void *ptr = pmm->alloc(sz);
    sz = (sz + 1) * 2 - 3;
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
