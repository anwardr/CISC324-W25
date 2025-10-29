// Mutex structure for user-space synchronization
struct mutex {
  int locked;           // Is the mutex locked?
  int pid;              // Process ID holding the mutex (0 if free)
  struct spinlock lk;   // Spinlock for protecting mutex operations
};

#define NMUTEX 64  // Maximum number of mutexes in the system
