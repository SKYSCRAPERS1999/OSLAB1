#include <os.h>
#include <mylib.h>
#include <fs.h>

//#define MTTEST
//#define SEMTEST
#define FILETEST

static void os_init();
static void os_run();
static _RegSet *os_interrupt(_Event ev, _RegSet *regs);
extern spinlock_t lock;

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
    for (volatile int i = 0, t = uptime(); uptime() - t < 100 ; i++);
    _putc('(');
    kmt->sem_signal(&fill);
  }
}
static void consumer() {
  while (1) {
    kmt->sem_wait(&fill);
    for (volatile int i = 0, t = uptime(); uptime() - t < 100 ; i++);
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

#ifdef FILETEST

char wbuf[256];
char rbuf[256];
char dir[256];
int fd[256];

static void simple_test(){
  for (int i = 0, ret; i < 100; i++){
    strcpy(wbuf, "Hello World "); strncat(wbuf, (char)(i + 'a'));
    strcpy(dir, "/Document/"); strncat(dir, (char)('a'+i%26)); strncat(dir, (char)('a'+i/26));

    fd[i] = vfs->open(dir, O_RDWR);
    ret = vfs->write(fd[i], wbuf, strlen(wbuf));  
    printf("wsiz = %d\n", ret);
    vfs->lseek(fd[i], 0, 0);
    ret = vfs->read(fd[i], rbuf, 20);
    printf("rsiz = %d\n", ret);
    printf("%s\n\n", rbuf); 
  }
  for (int i = 0; i < 100; i++) {
    vfs->close(fd[i]);
  }
}

static void test_file(){
  simple_test();
  

}

#endif

static void os_run() {

  #ifdef MTTEST
    test_run();
  #endif

  #ifdef SEMTEST
    test_sem();
  #endif

  #ifdef FILETEST
    test_file();
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

    if (ev.event == _EVENT_IRQ_TIMER) _putc('*');
    if (ev.event == _EVENT_IRQ_IODEV) _putc('I');
    if (ev.event == _EVENT_ERROR) _putc('x'),_halt(1);

  #endif

  if (t != NULL) return t->reg; // this is allowed by AM
  else return NULL;
}
