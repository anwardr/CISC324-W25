// This program demonstrates:
// - Task 1: Basic producer-consumer synchronization
// - Task 2: VIP priority queue implementation
// Note: The current implementation does not enforce synchronization
// Without proper synchronization, race conditions occur - makers might 
// overwrite pizzas, drivers might grab the same pizza, or the counter
// gets corrupted. The same happens in the handling of VIP customers.

// 
// Usage:
//   pizzadelivery 2 2        - Basic synchronization (Task 1)
//   pizzadelivery 2 2 vip    - With VIP priority (Task 2)



#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#define BUFFER_SIZE 10
#define ITEMS_PER_PRODUCER 10
#define BUFFER_FILE "buffervip"

// Pizza structure with VIP flag
struct pizza {
  int order_id;
  int is_vip;  // 1 for VIP, 0 for regular
};

// Shared buffer structure (shared between processes via file)
struct shared_buffer {
  struct pizza buffer[BUFFER_SIZE];  // Circular buffer of pizzas
  int in;                            // Next position to insert
  int out;                           // Next position to remove
  int count;                         // Current number of items
  
  // Synchronization primitives
  int mutex_id;                      // Protects buffer from concurrent access
  int empty_sem;                     // Counts empty slots (initially BUFFER_SIZE)
  int full_sem;                      // Counts full slots (initially 0)
  
  // Statistics
  int total_produced;
  int total_consumed;
  int vip_produced;
  int vip_consumed;
  
  // VIP mode flag (0 = Task 1, 1 = Task 2 with VIP priority)
  int use_vip_priority;
};

//=============================================================================
// HELPER FUNCTIONS: File-based sharing
//=============================================================================

void
read_buffer(struct shared_buffer *buf)
{
  int fd = open(BUFFER_FILE, O_RDONLY);
  if(fd < 0) {
    printf("ERROR: Cannot open buffer file for reading\n");
    exit(1);
  }
  read(fd, buf, sizeof(struct shared_buffer));
  close(fd);
}

void
write_buffer(struct shared_buffer *buf)
{
  int fd = open(BUFFER_FILE, O_WRONLY | O_CREATE);
  if(fd < 0) {
    printf("ERROR: Cannot open buffer file for writing\n");
    exit(1);
  }
  write(fd, buf, sizeof(struct shared_buffer));
  close(fd);
}

//=============================================================================
// PRODUCER LOGIC (Pizza Makers)
//=============================================================================

void
producer(int id)
{
  printf("Pizza Maker %d started\n", id);
  struct shared_buffer buf;
  
  for(int i = 0; i < ITEMS_PER_PRODUCER; i++) {
    // Create a pizza with unique order ID
    struct pizza pizza;
    pizza.order_id = id * 1000 + i;
    
    // Read current synchronization IDs from file
    read_buffer(&buf);

    // TODO #1: Write explanatory comment for the following block
    if(buf.use_vip_priority) {
      pizza.is_vip = (pizza.order_id % 10 < 3) ? 1 : 0;
    } else {
      pizza.is_vip = 0;  // Task 1: No VIP, all regular
    }
    
    //TODO #2: Wait for an empty slot
    
    //TODO #3: Lock the mutex to enter critical section
    
    // === CRITICAL SECTION ===
    // Read current buffer state
    read_buffer(&buf);
    
    // TODO #4: Write comment for the following block
    // to explain what it does

    buf.buffer[buf.in] = pizza;
    buf.in = (buf.in + 1) % BUFFER_SIZE;
    buf.count++;
    buf.total_produced++;
    
    if(buf.use_vip_priority && pizza.is_vip) {
      buf.vip_produced++;
      printf("  Maker %d: made VIP pizza #%d (holding area: %d/10) 🌟\n", 
             id, pizza.order_id, buf.count);
    } else {
      printf("  Maker %d: made pizza #%d (holding area: %d/10)\n", 
             id, pizza.order_id, buf.count);
    }
    
    // Write back modified buffer
    write_buffer(&buf);
    // === END CRITICAL SECTION ===
    
    //TODO #5: Unlock the mutex
    
    //TODO #6: Signal that there's now a full slot

    pause(5);  // Small delay for readability 
  }
  
  printf("Pizza Maker %d finished (%d pizzas)\n", id, ITEMS_PER_PRODUCER);
}

//=============================================================================
// CONSUMER LOGIC (Delivery Drivers) - WITH VIP PRIORITY
//=============================================================================

void
consumer(int id)
{
  printf("Delivery Driver %d started\n", id);
  struct shared_buffer buf;  // Local copy 
  
  for(int i = 0; i < ITEMS_PER_PRODUCER; i++) {
    
    // Read current synchronization IDs from file
    read_buffer(&buf);
    
    //TODO #7: Wait for a full slot (pizza available)
    
    //TODO #8: Lock the mutex to enter critical section
    
    // === CRITICAL SECTION ===
    // Read current buffer state
    read_buffer(&buf);
    
    struct pizza pizza;
    
    // TODO #9: Comment the following code block
    // to explain what it does
    if(buf.use_vip_priority) {
      // Search for VIP pizza first
      int vip_index = -1;
      int temp_index = buf.out;
      
      for(int j = 0; j < buf.count; j++) {
        if(buf.buffer[temp_index].is_vip) {
          vip_index = temp_index;
          break;
        }
        temp_index = (temp_index + 1) % BUFFER_SIZE;
      }
      
      if(vip_index >= 0) {
        // TODO #10: Comment what below code block does
        pizza = buf.buffer[vip_index];
        
        // Shift elements to fill the gap
        int current = vip_index;
        for(int j = 0; j < buf.count - 1; j++) {
          int next = (current + 1) % BUFFER_SIZE;
          buf.buffer[current] = buf.buffer[next];
          current = next;
        }
        
        // Adjust the 'in' pointer since we removed from middle
        buf.in = (buf.in - 1 + BUFFER_SIZE) % BUFFER_SIZE;
        buf.vip_consumed++;
        
        printf("  Driver %d: delivered VIP pizza #%d (holding area: %d/10) ⭐\n", 
               id, pizza.order_id, buf.count - 1);
      } else {
        // No VIP pizzas - deliver regular pizza from front
        pizza = buf.buffer[buf.out];
        buf.out = (buf.out + 1) % BUFFER_SIZE;
        
        printf("  Driver %d: delivered pizza #%d (holding area: %d/10)\n", 
               id, pizza.order_id, buf.count - 1);
      }
    } else {
      // Simple FIFO - no priority
      pizza = buf.buffer[buf.out];
      buf.out = (buf.out + 1) % BUFFER_SIZE;
      
      printf("  Driver %d: delivered pizza #%d (holding area: %d/10)\n", 
             id, pizza.order_id, buf.count - 1);
    }
    
    buf.count--;
    buf.total_consumed++;
    
    // Write back modified buffer
    write_buffer(&buf);
    // === END CRITICAL SECTION ===
    
    //TODO #11: Unlock the mutex
    
    //TODO #12: Signal that there's now an empty slot

    pause(5);  // Small delay for readability
  }
  
  printf("Delivery Driver %d finished (%d pizzas)\n", id, ITEMS_PER_PRODUCER);
}

//=============================================================================
// MAIN: Setup and Coordination
//=============================================================================

int
main(int argc, char *argv[])
{
  int num_producers = 2;
  int num_consumers = 2;
  struct shared_buffer buffer;  // Local buffer structure
  int use_vip = 0;
  
  // Parse command line arguments
  if(argc > 1) num_producers = atoi(argv[1]);
  if(argc > 2) num_consumers = atoi(argv[2]);
  if(argc > 3) use_vip = 1;  // Third argument enables VIP mode
  
  printf("\n===== 🍕 Pizza Delivery Service =====\n");
  
  if(use_vip) {
    printf("Mode: Task 2 - VIP Priority Enabled ⭐\n");
  } else {
    printf("Mode: Task 1 - Basic Synchronization\n");
  }
  
  printf("Holding area capacity: %d pizzas\n", BUFFER_SIZE);
  printf("Pizza makers: %d (each makes %d pizzas)\n", num_producers, ITEMS_PER_PRODUCER);
  printf("Delivery drivers: %d (each delivers %d pizzas)\n", num_consumers, ITEMS_PER_PRODUCER);
  
  if(use_vip) {
    printf("VIP rate: 30%% of all pizzas\n");
  }
  
  printf("======================================\n\n");
  
  // Initialize buffer structure
  buffer.in = 0;
  buffer.out = 0;
  buffer.count = 0;
  buffer.total_produced = 0;
  buffer.total_consumed = 0;
  buffer.vip_produced = 0;
  buffer.vip_consumed = 0;
  buffer.use_vip_priority = use_vip;
  
  // Create synchronization primitives
  buffer.mutex_id = mutex_create();
  buffer.empty_sem = sem_create(BUFFER_SIZE);  // Initially all slots empty
  buffer.full_sem = sem_create(0);             // Initially no slots full
  
  if(buffer.mutex_id < 0 || buffer.empty_sem < 0 || buffer.full_sem < 0) {
    printf("ERROR: Failed to create synchronization primitives\n");
    exit(1);
  }
  
  printf("Created mutex (ID=%d) and semaphores (empty=%d, full=%d)\n\n",
         buffer.mutex_id, buffer.empty_sem, buffer.full_sem);
  
  // Write initial buffer to file (this creates the shared state)
  unlink(BUFFER_FILE);  // Remove old file if exists
  write_buffer(&buffer);
  
  // Fork producer processes
  for(int i = 0; i < num_producers; i++) {
    int pid = fork();
    if(pid < 0) {
      printf("Fork failed\n");
      exit(1);
    }
    if(pid == 0) {
      // Child: run producer
      producer(i);
      exit(0);
    }
  }
  
  // Fork consumer processes  
  for(int i = 0; i < num_consumers; i++) {
    int pid = fork();
    if(pid < 0) {
      printf("Fork failed\n");
      exit(1);
    }
    if(pid == 0) {
      // Child: run consumer
      consumer(i);
      exit(0);
    }
  }
  
  // Wait for all children to finish
  int total_children = num_producers + num_consumers;
  for(int i = 0; i < total_children; i++) {
    wait(0);
  }
  
  // Read final buffer state from file
  read_buffer(&buffer);
  
  // Print final statistics
  printf("\n===== 📊 Final Results =====\n");
  printf("Total pizzas produced: %d\n", buffer.total_produced);
  printf("Total pizzas delivered: %d\n", buffer.total_consumed);
  
  if(buffer.use_vip_priority) {
    printf("VIP pizzas made: %d\n", buffer.vip_produced);
    printf("VIP pizzas delivered: %d\n", buffer.vip_consumed);
  }
  
  printf("Final holding area count: %d\n", buffer.count);
  
  int expected = num_producers * ITEMS_PER_PRODUCER;
  if(buffer.total_produced == expected && buffer.total_consumed == expected) {
    printf("✓ SUCCESS! All pizzas produced and delivered correctly.\n");
    if(buffer.use_vip_priority) {
      printf("  Task 2 Complete: VIP priority working! ⭐\n");
    } else {
      printf("  Task 1 Complete: Synchronization working!\n");
    }
  } else {
    printf("✗ ERROR: Mismatch detected!\n");
    printf("  Expected: %d produced and delivered\n", expected);
    printf("  Got: %d produced, %d delivered\n", 
           buffer.total_produced, buffer.total_consumed);
  }
  
  // Cleanup
  mutex_destroy(buffer.mutex_id);
  sem_destroy(buffer.empty_sem);
  sem_destroy(buffer.full_sem);
  unlink(BUFFER_FILE);  // Remove buffer file
  
  printf("============================\n");
  exit(0);
}
