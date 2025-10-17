# Plan 9 Kernel Architecture

## Overview

The Plan 9 kernel implements a minimal microkernel-style architecture focused on providing process management, memory management, and device abstraction through the file system interface.

## Kernel Structure

### Core Components

```mermaid
graph TB
    subgraph "Kernel Core"
        MAIN[main.c - Kernel Entry]
        PROC[proc.c - Process Management]  
        SCHED[sched.c - Scheduler]
        MEM[segment.c - Memory Management]
        TRAP[trap.c - Interrupt Handling]
    end
    
    subgraph "File System Layer"
        CHAN[chan.c - Channel Management]
        DEV[devtab.c - Device Table]
        MOUNT[mount.c - Mount Management] 
        NAMESPACE[pgrp.c - Process Groups/Namespaces]
    end
    
    subgraph "Device Drivers"
        DEVCON[devcons.c - Console]
        DEVROOT[devroot.c - Root Device]
        DEVPROC[devproc.c - /proc]
        DEVSRV[devsrv.c - Service Registry]
    end
    
    MAIN --> PROC
    PROC --> SCHED
    PROC --> MEM
    TRAP --> PROC
    
    PROC --> CHAN
    CHAN --> DEV
    DEV --> DEVCON
    DEV --> DEVROOT
    DEV --> DEVPROC
    DEV --> DEVSRV
    
    MOUNT --> NAMESPACE
    NAMESPACE --> CHAN
```

## Process Management

### Process States

```mermaid
stateDiagram-v2
    [*] --> Embryo: newproc()
    Embryo --> Ready: scheduler assigns
    Ready --> Running: dispatch()
    Running --> Ready: preempt/yield()
    Running --> Sleeping: sleep()
    Sleeping --> Ready: wakeup()
    Running --> Moribund: exit()
    Moribund --> Dead: reaper
    Dead --> [*]: freed
    
    Running --> Rendezvous: rendezvous()
    Rendezvous --> Ready: partner arrives
    
    Running --> Stopped: debug stop
    Stopped --> Ready: continue
```

## Memory Management

### Address Space Layout

```mermaid
graph TB
    subgraph "Virtual Address Space (per process)"
        direction TB
        USTKTOP[0x80000000 - User Stack Top]
        USTACK[User Stack - grows down]
        GAP1[Unmapped Gap]
        HEAP[Heap - grows up] 
        BSS[BSS Segment]
        DATA[Data Segment]
        TEXT[Text Segment]
        UZERO[0x00000000 - UZERO]
    end
    
    subgraph "Kernel Address Space"
        KZERO[KZERO - Kernel Base]
        KERNEL[Kernel Text/Data]
        KSTACK[Kernel Stacks]
        DEVMEM[Device Memory Maps]
        PHYSMEM[Physical Memory Map]
    end
```

## Device Management

### Device Interface

```mermaid
graph LR
    subgraph "System Calls"
        OPEN[open()]
        READ[read()]
        WRITE[write()]
        CLOSE[close()]
    end
    
    subgraph "Name Resolution"
        WALK[namec()]
        CHAN[Channel]
        CDEV[cdevsw[]]
    end
    
    subgraph "Device Operations"
        DEVOPEN[devopen()]
        DEVREAD[devread()]
        DEVWRITE[devwrite()]
        DEVCLOSE[devclose()]
    end
    
    OPEN --> WALK
    READ --> CHAN
    WRITE --> CHAN  
    CLOSE --> CHAN
    
    WALK --> CHAN
    CHAN --> CDEV
    CDEV --> DEVOPEN
    CDEV --> DEVREAD
    CDEV --> DEVWRITE
    CDEV --> DEVCLOSE
```

## Scheduling

### Scheduler Algorithm

```mermaid
graph TB
    subgraph "Scheduler Loop"
        START[Scheduler Start]
        FIND[Find Runnable Process]
        CHECK[Priority Check]
        SWITCH[Context Switch]
        RUN[Process Runs]
        PREEMPT[Preemption/Yield]
    end
    
    subgraph "Process Queues"
        RUNQ[Run Queue]
        SLEEPQ[Sleep Queue]  
        RENDEZQ[Rendezvous Queue]
    end
    
    START --> FIND
    FIND --> RUNQ
    FIND --> CHECK
    CHECK --> SWITCH
    SWITCH --> RUN
    RUN --> PREEMPT
    PREEMPT --> RUNQ
    PREEMPT --> SLEEPQ
    PREEMPT --> RENDEZQ
    RUNQ --> FIND
```

## System Calls

### System Call Interface

```mermaid
sequenceDiagram
    participant User as User Process
    participant Trap as Trap Handler
    participant Syscall as System Call
    participant Kernel as Kernel Function
    
    User->>Trap: syscall instruction
    Trap->>Syscall: dispatch by number
    Syscall->>Kernel: call implementation
    Kernel->>Syscall: return result
    Syscall->>Trap: set return value
    Trap->>User: return to user mode
```

## References

- `/sys/src/9/port/` - Portable kernel source
- `/sys/src/9/pc/` - PC-specific kernel code  
- `/sys/man/2/` - System call manual pages
- `/sys/man/3/` - Device driver manual pages