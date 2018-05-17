#ifndef __OS_H__
#define __OS_H__

//#define __LOCAL_DEBUG__
#ifdef __LOCAL_DEBUG__
# define _debug(fmt, ...) printf("\033[94m%s: " fmt "\n\033[0m", \
    __func__, ##__VA_ARGS__)
#else
# define _debug(...) ((void) 0)
#endif

#define MAXTRD 20
#define STKSZ (1<<10)
#define REGSZ (1<<7)
#define SEMBUFSZ 3

#include <kernel.h>

struct spinlock {
  int locked;       // Is the lock held?
  const char *name;        // Name of lock.
};

struct thread {
  int id;
  int freed; //0, 1
  void *kstack;
  _RegSet *reg;
};

struct semaphore {
	int id;
	int count;
	const char *name; 
};

static inline void puts(const char *p) {
  for (; *p; p++) {
    _putc(*p);
  }
}

#endif
