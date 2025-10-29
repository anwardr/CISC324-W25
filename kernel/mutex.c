// Mutex implementation for xv6
// Provides user-space mutex (mutual exclusion locks) functionality

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "mutex.h"

// Global array of mutexes
struct {
  struct spinlock lock;
  struct mutex mutexes[NMUTEX];
} mutex_table;

// Initialize the mutex table
void
mutexinit(void)
{
  initlock(&mutex_table.lock, "mutex_table");
  for(int i = 0; i < NMUTEX; i++) {
    mutex_table.mutexes[i].locked = 0;
    mutex_table.mutexes[i].pid = 0;
    initlock(&mutex_table.mutexes[i].lk, "mutex");
  }
}

// Create a new mutex and return its ID
// Returns mutex ID on success, -1 on failure
int
mutex_create(void)
{
  acquire(&mutex_table.lock);
  
  // Find an unused mutex slot
  for(int i = 0; i < NMUTEX; i++) {
    if(mutex_table.mutexes[i].pid == 0 && mutex_table.mutexes[i].locked == 0) {
      mutex_table.mutexes[i].locked = 0;
      mutex_table.mutexes[i].pid = -1; // Mark as allocated but not locked
      release(&mutex_table.lock);
      return i;
    }
  }
  
  release(&mutex_table.lock);
  return -1; // No free mutex slots
}

// Destroy a mutex
// Returns 0 on success, -1 on failure
int
mutex_destroy(int mutex_id)
{
  if(mutex_id < 0 || mutex_id >= NMUTEX)
    return -1;
  
  acquire(&mutex_table.lock);
  
  struct mutex *m = &mutex_table.mutexes[mutex_id];
  
  // Cannot destroy a locked mutex
  if(m->locked) {
    release(&mutex_table.lock);
    return -1;
  }
  
  m->pid = 0;
  m->locked = 0;
  
  release(&mutex_table.lock);
  return 0;
}

// Lock a mutex (blocking operation)
// Returns 0 on success, -1 on failure
int
mutex_lock(int mutex_id)
{
  struct proc *p = myproc();
  
  if(mutex_id < 0 || mutex_id >= NMUTEX)
    return -1;
  
  struct mutex *m = &mutex_table.mutexes[mutex_id];
  
  acquire(&m->lk);
  
  // Wait while the mutex is locked by another process
  while(m->locked) {
    // Sleep until the mutex is released
    sleep(m, &m->lk);
  }
  
  // Acquire the mutex
  m->locked = 1;
  m->pid = p->pid;
  
  release(&m->lk);
  return 0;
}

// Unlock a mutex
// Returns 0 on success, -1 on failure
int
mutex_unlock(int mutex_id)
{
  struct proc *p = myproc();
  
  if(mutex_id < 0 || mutex_id >= NMUTEX)
    return -1;
  
  struct mutex *m = &mutex_table.mutexes[mutex_id];
  
  acquire(&m->lk);
  
  // Only the process that locked the mutex can unlock it
  if(!m->locked || m->pid != p->pid) {
    release(&m->lk);
    return -1;
  }
  
  // Release the mutex
  m->locked = 0;
  m->pid = 0;
  
  // Wake up one waiting process
  wakeup(m);
  
  release(&m->lk);
  return 0;
}
