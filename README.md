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
| `AD`            | Add (R1 ← R1 + memory).                           |
| `SB`            | Subtract (R1 ← R1 - memory).                      |
| `ML`            | Multiply (R1 ← R1 * memory).                      |
| `DV`            | Divide (R1 ← quotient, R2 ← remainder).           |
| `BC`            | Branch on carry.                                  |
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