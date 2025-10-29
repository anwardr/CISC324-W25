// Semaphore implementation for xv6
// Provides counting semaphore functionality for user-space synchronization

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "semaphore.h"

// Global array of semaphores
struct {
  struct spinlock lock;
  struct semaphore semaphores[NSEM];
} sem_table;

// Initialize the semaphore table
void
seminit(void)
{
  initlock(&sem_table.lock, "sem_table");
  for(int i = 0; i < NSEM; i++) {
    sem_table.semaphores[i].value = 0;
    sem_table.semaphores[i].used = 0;
    initlock(&sem_table.semaphores[i].lk, "semaphore");
  }
}

// Create a new semaphore with initial value
// Returns semaphore ID on success, -1 on failure
int
sem_create(int initial_value)
{
  if(initial_value < 0)
    return -1;
  
  acquire(&sem_table.lock);
  
  // Find an unused semaphore slot
  for(int i = 0; i < NSEM; i++) {
    if(sem_table.semaphores[i].used == 0) {
      sem_table.semaphores[i].value = initial_value;
      sem_table.semaphores[i].used = 1;
      release(&sem_table.lock);
      return i;
    }
  }
  
  release(&sem_table.lock);
  return -1; // No free semaphore slots
}

// Destroy a semaphore
// Returns 0 on success, -1 on failure
int
sem_destroy(int sem_id)
{
  if(sem_id < 0 || sem_id >= NSEM)
    return -1;
  
  acquire(&sem_table.lock);
  
  struct semaphore *s = &sem_table.semaphores[sem_id];
  
  if(!s->used) {
    release(&sem_table.lock);
    return -1;
  }
  
  s->used = 0;
  s->value = 0;
  
  release(&sem_table.lock);
  return 0;
}

// Wait (P operation / down operation)
// Decrements the semaphore value, blocks if value is 0
// Returns 0 on success, -1 on failure
int
sem_wait(int sem_id)
{
  if(sem_id < 0 || sem_id >= NSEM)
    return -1;
  
  struct semaphore *s = &sem_table.semaphores[sem_id];
  
  acquire(&s->lk);
  
  if(!s->used) {
    release(&s->lk);
    return -1;
  }
  
  // Wait while semaphore value is 0
  while(s->value == 0) {
    sleep(s, &s->lk);
    
    // Check if semaphore was destroyed while sleeping
    if(!s->used) {
      release(&s->lk);
      return -1;
    }
  }
  
  // Decrement the semaphore value
  s->value--;
  
  release(&s->lk);
  return 0;
}

// Signal (V operation / up operation)
// Increments the semaphore value and wakes up a waiting process
// Returns 0 on success, -1 on failure
int
sem_signal(int sem_id)
{
  if(sem_id < 0 || sem_id >= NSEM)
    return -1;
  
  struct semaphore *s = &sem_table.semaphores[sem_id];
  
  acquire(&s->lk);
  
  if(!s->used) {
    release(&s->lk);
    return -1;
  }
  
  // Increment the semaphore value
  s->value++;
  
  // Wake up one waiting process
  wakeup(s);
  
  release(&s->lk);
  return 0;
}
