# CISC 324 - Assignment 3: Synchronization in xv6-riscv

**Due Date:** Nov. 10, 2025

**Weight:** 100 points

## Repository Setup

Before starting the assignment, you need to clone the `feature/a3` branch of the repository:

```bash
git clone --branch feature/a3 --single-branch https://github.com/anwardr/CISC324-F25.git
```

After cloning, navigate to the repository directory:

```bash
cd CISC324-F25
```

---

## Assignment Overview

This assignment has **two parts** focusing on synchronization mechanisms (mutexes and semaphores) in the xv6-riscv operating system.

---

## Part 1: Explore Basic Mutex Functionality in xv6-riscv (25%)

### What is Given / Tasks To Do

A boilerplate code file [`counter.c`](user/counter.c) is provided in the `user/` directory.

#### Step 1: Run Without Mutex

**Run the program without mutex** to observe the behavior without proper synchronization:
```bash
make clean && make qemu
# In the xv6 shell:
counter
```

**Record** the program output with your explanation in a file named `assignment3_Last4Digit_ID.pdf` in the project directory.

#### Step 2: Complete the TODOs

**Fill in the TODO** sections with appropriate lines of code to implement proper mutex-based synchronization.

#### Step 3: Test Your Implementation

**Run the program with mutex** to observe synchronized behavior after completing the TODOs:
```bash
make clean && make qemu
# In the xv6 shell:
counter mutex
```

**Record** the program output with your explanation in the same `assignment3_Last4Digit_ID.pdf` file, including:
- Output when running `counter` (without mutex) from Step 1
- Explanation of the TODO implementations
- Output when running `counter mutex` (with mutex) after implementing TODOs
- Explanation of what changed and why the mutex implementation fixes the synchronization issues

---

## Part 2: Modify Pizza Delivery Service Simulation (60%)

### Overview

You will modify an existing Pizza Delivery Service Simulation system that demonstrates producer-consumer synchronization problems and their solutions.

---

### Task 1: Basic Pizza Shop Synchronization (20%)

#### Problem Description

A pizza shop has a holding area (circular buffer) with capacity for **10 pizzas**. The system has:
- **Multiple pizza makers (producers)** who create pizzas and place them in the holding area
- **Multiple delivery drivers (consumers)** who pick up pizzas and deliver them

**Without proper synchronization**, race conditions occur:
- Makers might overwrite pizzas
- Drivers might grab the same pizza
- The counter gets corrupted

#### Goal

Use **mutexes** and **semaphores** to synchronize access so that:
- Makers wait when the holding area is full
- Drivers wait when the holding area is empty
- Only one person accesses the holding area at a time
- All pizzas are correctly produced and delivered (no lost pizzas!)

---

### Task 2: VIP Priority Delivery (40%)

#### Problem Description

The pizza shop now has **VIP customers** (30% of orders). Currently, drivers deliver pizzas in the order they were made (FIFO - First In, First Out). However, VIP customers expect faster service - their pizzas should be delivered first, even if regular pizzas were made earlier.

#### Goal

Modify the consumer logic to implement **priority delivery**:
- When a driver enters the holding area, search for VIP pizzas first
- If a VIP pizza exists, deliver it (remove from middle of queue)
- If no VIP pizzas, deliver the oldest regular pizza (normal FIFO)
- Maintain all synchronization from Task 1
- Properly shift the circular buffer when removing from the middle

---

### What is Given / Tasks To Do

A boilerplate code file named [`pizzadelivery.c`](user/pizzadelivery.c) is provided in the `user/` directory.

#### Step 1: Run the Original Program

Run the `pizzadelivery` program in **two modes**:

1. **Without VIP option:**
   ```bash
   make qemu
   # In the xv6 shell:
   pizzadelivery 2 2
   ```

2. **With VIP option:**
   ```bash
   make qemu
   # In the xv6 shell:
   pizzadelivery 2 2 vip
   ```

**Record** the program output for both modes with your explanation in the same `assignment3_Last4Digit_ID.pdf` file.

#### Step 2: Complete the TODOs

Several `TODO` sections have been defined within the `pizzadelivery.c` code. You need to:
1. Locate all TODO comments in the code
2. Implement the required functionality for each TODO
3. Ensure proper synchronization and priority handling

#### Step 3: Test Your Implementation

After addressing all the TODOs, run the code again in **both modes**:

1. **Without VIP option:**
   ```bash
   make qemu
   # In the xv6 shell:
   pizzadelivery
   ```

2. **With VIP option:**
   ```bash
   make qemu
   # In the xv6 shell:
   pizzadelivery vip
   ```

**Record** the program output for both modes with your explanation in the same `assignment3_Last4Digit_ID.pdf` file.

---

## Files to Submit

You must submit the following:

1. **Submit the overall compressed xv6-riscv folder**
   - Make sure to run `make clean` before compressing to reduce file size

2. **Assignment report PDF file named `assignment3_Last4Digit_ID.pdf`**

---

## Marking Criteria

| Component | Weight |
|-----------|--------|
| **Part 1:** Basic Mutex Functionality | 25% |
| **Part 2 - Task 1:** Basic Pizza Shop Synchronization | 20% |
| **Part 2 - Task 2:** VIP Priority Delivery | 40% |
| **Overall Quality of Submission** | 15% |
| **Total** | **100%** |
