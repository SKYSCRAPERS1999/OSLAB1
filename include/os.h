#ifndef __OS_H__
#define __OS_H__

#define MAXTRD 20
#define STKSZ (1<<12)
#define REGSZ (1<<6)

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
