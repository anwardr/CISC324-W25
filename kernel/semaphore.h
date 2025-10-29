// Semaphore structure for user-space synchronization
struct semaphore {
  int value;            // Semaphore value (can be 0 or positive)
  int used;             // Is this semaphore slot in use?
  struct spinlock lk;   // Spinlock for protecting semaphore operations
};

#define NSEM 64  // Maximum number of semaphores in the system
