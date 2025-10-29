// counter.c - SIMPLIFIED Shared Counter Program
// Demonstrates race conditions and how mutex fixes them
//
// STUDENT INSTRUCTIONS:
// 1. First run: counter (without mutex) - see the BUG
// 2. Complete the 3 required TODOs (plus 1 optional)
// 3. Run: counter mutex - see it FIXED!

#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#define NUM_PROCESSES 4
#define INCREMENTS_PER_PROCESS 1000
#define EXPECTED_TOTAL (NUM_PROCESSES * INCREMENTS_PER_PROCESS)
#define COUNTER_FILE "counter.dat"

int mutex_id = -1;  // Will hold our mutex ID

// Read counter from file
int
read_counter(void)
{
  int fd = open(COUNTER_FILE, O_RDONLY);
  if(fd < 0) return 0;
  
  int value = 0;
  read(fd, &value, sizeof(int));
  close(fd);
  return value;
}

// Write counter to file
void
write_counter(int value)
{
  int fd = open(COUNTER_FILE, O_CREATE | O_WRONLY);
  if(fd < 0) {
    printf("Failed to open counter file\n");
    exit(1);
  }
  write(fd, &value, sizeof(int));
  close(fd);
}

void
increment_counter(int use_mutex, int process_id)
{
  for(int i = 0; i < INCREMENTS_PER_PROCESS; i++) {
    
    // ========== TODO #1 ==========
    // TODO: Enable the lock
    //HINT: If using mutex, LOCK it here before entering critical section
    //HINT: acquire lock by calling mutex lock function as mutex_lock(mutex_id)

    // === CRITICAL SECTION ===
    // This code accesses shared data and causes race conditions!
    int temp = read_counter();
    temp = temp + 1;
    write_counter(temp);
    // === END CRITICAL SECTION ===
    
    // ========== TODO #2 ==========
    // TODO: Release the lock
    // HINT: If using mutex, use mutex_unlock function to release the lock 
  }
  
  printf("Process %d done\n", process_id);
}

int
main(int argc, char *argv[])
{
  int use_mutex = 0;
  
  // Check if user wants to use mutex
  if(argc > 1 && strcmp(argv[1], "mutex") == 0) {
    use_mutex = 1;
  }
  
  printf("\n===== Shared Counter Test =====\n");
  printf("Processes: %d\n", NUM_PROCESSES);
  printf("Increments per process: %d\n", INCREMENTS_PER_PROCESS);
  printf("Expected final value: %d\n", EXPECTED_TOTAL);
  printf("Using mutex: %s\n", use_mutex ? "YES" : "NO");
  printf("================================\n\n");
  
  // Initialize counter file
  unlink(COUNTER_FILE);  // Remove old file if exists
  write_counter(0);      // Start with 0
  
  if(use_mutex) {
    mutex_id = mutex_create();
    if(mutex_id < 0) {
      printf("ERROR: Failed to create mutex\n");
      exit(1);
    }
    printf("Mutex created (ID = %d)\n\n", mutex_id);
  }
  
  // Fork child processes
  for(int i = 0; i < NUM_PROCESSES; i++) {
    int pid = fork();
    if(pid < 0) {
      printf("Fork failed\n");
      exit(1);
    }
    if(pid == 0) {
      // Child process
      increment_counter(use_mutex, i);
      exit(0);
    }
  }
  
  // Wait for all children
  for(int i = 0; i < NUM_PROCESSES; i++) {
    wait(0);
  }
  
  // Check results
  int final_value = read_counter();
  
  printf("\n===== RESULTS =====\n");
  printf("Final counter value: %d\n", final_value);
  printf("Expected value: %d\n", EXPECTED_TOTAL);
  
  if(final_value == EXPECTED_TOTAL) {
    printf("✓ SUCCESS! Counter is correct.\n");
  } else {
    printf("✗ FAILURE! Race condition occurred!\n");
    printf("  Lost updates: %d\n", EXPECTED_TOTAL - final_value);
  }
  printf("===================\n\n");
  
  if(use_mutex) mutex_destroy(mutex_id);
  
  // Cleanup
  unlink(COUNTER_FILE);
  
  exit(0);
}
