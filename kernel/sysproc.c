#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;

  argint(0, &n);
  argint(1, &t);
  addr = myproc()->sz;

  if(t == SBRK_EAGER || n < 0) {
    if(growproc(n) < 0) {
      return -1;
    }
  } else {
    // Lazily allocate memory for this process: increase its memory
    // size but don't allocate memory. If the processes uses the
    // memory, vmfault() will allocate it.
    if(addr + n < addr)
      return -1;
    if(addr + n > TRAPFRAME)
      return -1;
    myproc()->sz += n;
  }
  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kkill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// System call wrappers for mutex operations
uint64
sys_mutex_create(void)
{
  return mutex_create();
}

uint64
sys_mutex_destroy(void)
{
  int mutex_id;
  argint(0, &mutex_id);
  return mutex_destroy(mutex_id);
}

uint64
sys_mutex_lock(void)
{
  int mutex_id;
  argint(0, &mutex_id);
  return mutex_lock(mutex_id);
}

uint64
sys_mutex_unlock(void)
{
  int mutex_id;
  argint(0, &mutex_id);
  return mutex_unlock(mutex_id);
}

// System call wrappers for semaphore operations
uint64
sys_sem_create(void)
{
  int initial_value;
  argint(0, &initial_value);
  return sem_create(initial_value);
}

uint64
sys_sem_destroy(void)
{
  int sem_id;
  argint(0, &sem_id);
  return sem_destroy(sem_id);
}

uint64
sys_sem_wait(void)
{
  int sem_id;
  argint(0, &sem_id);
  return sem_wait(sem_id);
}

uint64
sys_sem_signal(void)
{
  int sem_id;
  argint(0, &sem_id);
  return sem_signal(sem_id);
}
