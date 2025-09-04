# Virtual OS Project

This project is a simple virtual operating system designed to help understand how real operating systems work under the hood. The project is divided into multiple phases, with **Phase 2** completed and work on **Phase 3** currently in progress.

---

## Project Overview

The virtual OS simulates an old-style batch operating system that processes jobs from input files. Each job consists of control cards, program cards, and data cards. The system executes these jobs using a custom instruction set and implements features like memory management and error handling.

---

## Phases

### **Phase 1: Job Loading and Execution**
- Focuses on loading and executing jobs.
- Jobs are represented as a group of cards:
    - **Control Cards**: `$AMJ`, `$DTA`, `$END`.
    - **Program Cards**: Contain instructions (10 instructions per 40-byte card).
    - **Data Cards**: Provide input data for the program.
- Example Job:
    ```
    $AMJ000120000200
    GD90LR92SR80PD80DM90LR93SR80PD80DM90LR92
    AD93SR80PD80LR93SR92LR80SR93DM90LR90CR91
    BT23CRR1BT09H    
    $DTA
    0033000000000001
    $END0001
    ```
- Key Features:
    - Direct access to physical memory.
    - Instructions address memory within a 0-100 word range, extended using segment registers.

---

### **Phase 2: Error Handling and Memory Management**
- Introduced paging for mapping virtual addresses to physical memory.
- Implemented a **Process Control Block (PCB)** to store job details.
- Enhanced error handling mechanisms.
- Still operates in a single-programming mode (no multiprogramming yet).

---

### **Phase 3: Multiprogramming (In Progress)**
- Focuses on enabling multiprogramming.
- Will introduce scheduling and resource management.

TODOS for Phase 3:
- Fix loader to handle multiple jobs.
    - if state is 0 skip pass everything else except $AMJ [done]
    - $DTA will not invoke Execute, it will put job in the ready queue [done]
    - $END will be handled gracefully [done]
    - fix init, stop clearing all memory [done]
    - fix value of DS in init, it should be +1 that max(CS) for that program, insted of setting 1 by default. [done]
    - a way to free allocated memory after HALT [done]
- PCB
    - add way to store infile & outfile SEEK location in PCB [done]
    - add way to store complete CPU state in the PCB [done]
    - support for context switching [done]
    - fix unbalanced sizes (fist page has 5 ptrs, next will have 9) [done]
- Execution
    - Add another conditions to break execution loop, according to schedular
    - Schedular will call execute after context switching next candidate
    - fix resolving addess from CS & IC, it should be fetchAddress instded for resolving page fault, it should be invalid page fault
- Extra (Next Phase)
    - complete asambler & linker
    - add logging
    - upgrade MemDump
    - refactor code
    - build compiler

---

## Instruction Set

The virtual OS supports the following instructions:

| **Instruction** | **Description**                                   |
|------------------|---------------------------------------------------|
| `GD`            | Get data (reads a data card).                     |
| `PD`            | Print data (prints a 40-byte block).              |
| `H`             | Halt execution.                                   |
| `LR`            | Load register (load memory content into R1).      |
| `SR`            | Store register (store R1 content into memory).    |
| `CR`            | Compare register (compare memory and R1 content). |
| `BT`            | Branch if true (jump if condition flag is set).   |
| `JT`            | Branch if true (uses R1 as segment register).     |
| `AD`            | Add (R1 ← R1 + memory).                           |
| `SB`            | Subtract (R1 ← R1 - memory).                      |
| `ML`            | Multiply (R1 ← R1 * memory).                      |
| `DV`            | Divide (R1 ← quotient, R2 ← remainder).           |
| `BC`            | Branch on carry.                                  |
| `JC`            | Branch on carry (uses R1 as segment register).    |
| `AN`            | Logical AND.                                      |
| `OR`            | Logical OR.                                       |
| `NT`            | Logical NOT.                                      |
| `LS`            | Left shift.                                       |
| `RS`            | Right shift.                                      |

---

## Future Work

- Complete **Phase 3** with multiprogramming support.
- Add advanced scheduling algorithms.
- Enhance debugging and logging features.

---  