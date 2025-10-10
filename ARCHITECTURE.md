# Plan 9 from Bell Labs - Technical Architecture

## Table of Contents
1. [System Overview](#system-overview)
2. [Kernel Architecture](#kernel-architecture)
3. [9P Protocol](#9p-protocol)
4. [File Systems](#file-systems)
5. [Authentication & Security](#authentication--security)
6. [Networking](#networking)
7. [User Space](#user-space)
8. [Development Environment](#development-environment)

## System Overview

Plan 9 is a distributed operating system designed at Bell Labs as a research successor to Unix. It extends the "everything is a file" philosophy to network resources, creating a unified namespace that spans multiple machines.

### Core Design Principles

```mermaid
graph TB
    subgraph "Plan 9 Core Principles"
        A[Everything is a File] --> B[9P Protocol]
        C[Unified Namespace] --> D[Network Transparency]
        E[Per-Process Namespace] --> F[Resource Isolation]
        G[Simple Interfaces] --> H[Composability]
    end
    
    B --> I[Remote Resources]
    D --> J[Distributed Computing]
    F --> K[Security Model]
    H --> L[Tool Ecosystem]
```

### System Architecture Overview

```mermaid
graph TB
    subgraph "CPU Server"
        CPU[CPU Service]
        CPUMEM[Memory Management]
        CPUPROC[Process Management]
    end
    
    subgraph "File Server"
        FS[File System Service]
        FOSSIL[Fossil File System]
        VENTI[Venti Archive Storage]
    end
    
    subgraph "Authentication Server"
        AUTH[Authentication Service]
        FACTOTUM[Factotum Agent]
        SECSTORE[Secstore]
    end
    
    subgraph "Terminal"
        TERM[Terminal/Workstation]
        RIO[Rio Window Manager]
        ACME[Acme Editor]
    end
    
    subgraph "Network Infrastructure"
        NINEP[9P Protocol]
        IL[IL Protocol]
        TCP[TCP/IP Stack]
    end
    
    CPU -->|9P| NINEP
    FS -->|9P| NINEP
    AUTH -->|9P| NINEP
    TERM -->|9P| NINEP
    
    NINEP --> IL
    NINEP --> TCP
    
    FS --> FOSSIL
    FOSSIL --> VENTI
    AUTH --> FACTOTUM
    AUTH --> SECSTORE
    TERM --> RIO
    TERM --> ACME
```

## Kernel Architecture

Plan 9's kernel is designed around the concept of resources as files and processes as the primary abstraction for computation.

### Kernel Structure

```mermaid
graph TB
    subgraph "Kernel Space"
        SCHED[Scheduler]
        MM[Memory Manager]
        PROC[Process Manager]
        DEV[Device Drivers]
        NAMESPACE[Namespace Manager]
        SYSCALL[System Call Interface]
    end
    
    subgraph "User Space"
        USER[User Processes]
        LIB9P[lib9p]
        LIBC[libc]
    end
    
    USER --> SYSCALL
    SYSCALL --> PROC
    SYSCALL --> MM
    SYSCALL --> NAMESPACE
    
    PROC --> SCHED
    MM --> SCHED
    DEV --> NAMESPACE
    
    USER --> LIB9P
    USER --> LIBC
```

### Process Model

```mermaid
stateDiagram-v2
    [*] --> New
    New --> Ready: proc()
    Ready --> Running: schedule()
    Running --> Ready: preempt()
    Running --> Blocked: sleep()
    Blocked --> Ready: wakeup()
    Running --> Zombie: exit()
    Zombie --> [*]: wait()
    
    Running --> Rendezvous: rendezvous()
    Rendezvous --> Ready: rendezvous_complete()
```

### Memory Management

```mermaid
graph TB
    subgraph "Virtual Address Space"
        TEXT[Text Segment]
        DATA[Data Segment]
        BSS[BSS Segment]
        HEAP[Heap]
        STACK[Stack]
    end
    
    subgraph "Physical Memory"
        KERNEL[Kernel Memory]
        USER[User Memory]
        CACHE[Page Cache]
        FREE[Free Pages]
    end
    
    TEXT --> USER
    DATA --> USER
    BSS --> USER
    HEAP --> USER
    STACK --> USER
    
    USER --> CACHE
    CACHE --> FREE
```

## 9P Protocol

The 9P protocol is the heart of Plan 9's distributed architecture, providing a standard way to access resources over the network.

### 9P Message Flow

```mermaid
sequenceDiagram
    participant Client
    participant Server
    
    Note over Client,Server: Connection Establishment
    Client->>Server: Tversion
    Server->>Client: Rversion
    
    Note over Client,Server: Authentication (if required)
    Client->>Server: Tauth
    Server->>Client: Rauth
    
    Note over Client,Server: Attach to File System
    Client->>Server: Tattach
    Server->>Client: Rattach
    
    Note over Client,Server: Navigate and Access Files
    Client->>Server: Twalk
    Server->>Client: Rwalk
    
    Client->>Server: Topen
    Server->>Client: Ropen
    
    Client->>Server: Tread/Twrite
    Server->>Client: Rread/Rwrite
    
    Client->>Server: Tclunk
    Server->>Client: Rclunk
```

### 9P Protocol Stack

```mermaid
graph TB
    subgraph "Application Layer"
        APP[Applications]
        SHELL[Shell/rc]
        EDITOR[Text Editors]
    end
    
    subgraph "9P Layer"
        MOUNT[Mount Points]
        NAMESPACE[Namespace]
        NINEP[9P Protocol]
    end
    
    subgraph "Transport Layer"
        IL[IL Protocol]
        TCP[TCP]
        PIPE[Pipes]
    end
    
    subgraph "Network Layer"
        IP[IP]
        ETHER[Ethernet]
    end
    
    APP --> MOUNT
    SHELL --> MOUNT
    EDITOR --> MOUNT
    
    MOUNT --> NAMESPACE
    NAMESPACE --> NINEP
    
    NINEP --> IL
    NINEP --> TCP
    NINEP --> PIPE
    
    IL --> IP
    TCP --> IP
    IP --> ETHER
```

## File Systems

Plan 9 features a sophisticated file system architecture with Fossil providing the active file system and Venti providing archival storage.

### File System Architecture

```mermaid
graph TB
    subgraph "Active Storage"
        FOSSIL[Fossil File System]
        CACHE[Block Cache]
        TEMP[Temporary Storage]
    end
    
    subgraph "Archival Storage"
        VENTI[Venti Server]
        BLOCKS[Content-Addressed Blocks]
        INDEX[Block Index]
    end
    
    subgraph "Client Interface"
        NINEP[9P Interface]
        MOUNT[Mount Points]
        NAMESPACE[Per-Process Namespace]
    end
    
    NINEP --> FOSSIL
    FOSSIL --> CACHE
    FOSSIL --> TEMP
    FOSSIL -->|Snapshot| VENTI
    VENTI --> BLOCKS
    VENTI --> INDEX
    
    MOUNT --> NINEP
    NAMESPACE --> MOUNT
```

### Fossil Block Structure

```mermaid
graph TB
    subgraph "Fossil Block Types"
        ROOT[Root Block]
        DIR[Directory Block]
        DATA[Data Block]
        POINTER[Pointer Block]
    end
    
    subgraph "Venti Storage"
        SCORE[SHA1 Scores]
        ARENA[Storage Arenas]
        LOG[Write Log]
    end
    
    ROOT --> DIR
    DIR --> DATA
    DIR --> POINTER
    POINTER --> DATA
    
    ROOT -->|backup| SCORE
    DIR -->|backup| SCORE
    DATA -->|backup| SCORE
    POINTER -->|backup| SCORE
    
    SCORE --> ARENA
    ARENA --> LOG
```

## Authentication & Security

Plan 9's security model is based on capability-style authentication using the factotum agent and a centralized authentication server.

### Authentication Architecture

```mermaid
graph TB
    subgraph "Authentication Server"
        AS[Auth Server]
        AUTHDB[User Database]
        KEYFS[Key File System]
    end
    
    subgraph "Client Machine"
        FACTOTUM[Factotum Agent]
        SECSTORE[Secstore Client]
        PROCESS[User Process]
    end
    
    subgraph "File Server"
        FS[File Server]
        CHALLENGE[Challenge/Response]
    end
    
    PROCESS -->|auth request| FACTOTUM
    FACTOTUM -->|negotiate| AS
    AS --> AUTHDB
    AS --> KEYFS
    
    FACTOTUM -->|credentials| FS
    FS --> CHALLENGE
    
    FACTOTUM --> SECSTORE
    SECSTORE --> AS
```

### Security Protocol Flow

```mermaid
sequenceDiagram
    participant User
    participant Factotum
    participant AuthServer
    participant FileServer
    
    User->>Factotum: Request access
    Factotum->>AuthServer: Authenticate user
    AuthServer->>Factotum: Return ticket + authenticator
    Factotum->>FileServer: Present credentials
    FileServer->>AuthServer: Verify authenticator
    AuthServer->>FileServer: Confirmation
    FileServer->>User: Grant access
```

## Networking

Plan 9's networking stack supports both its native IL protocol and standard TCP/IP, with special support for the 9P protocol.

### Network Stack

```mermaid
graph TB
    subgraph "Applications"
        NINEP[9P Protocol]
        HTTP[Web Services]
        MAIL[Mail System]
    end
    
    subgraph "Transport Protocols"
        IL[IL Protocol]
        TCP[TCP]
        UDP[UDP]
    end
    
    subgraph "Network Layer"
        IP[IP Layer]
        ICMP[ICMP]
        ARP[ARP]
    end
    
    subgraph "Data Link"
        ETHER[Ethernet]
        PPP[PPP]
        LOOPBACK[Loopback]
    end
    
    NINEP --> IL
    NINEP --> TCP
    HTTP --> TCP
    MAIL --> TCP
    
    IL --> IP
    TCP --> IP
    UDP --> IP
    
    IP --> ICMP
    IP --> ARP
    IP --> ETHER
    IP --> PPP
    IP --> LOOPBACK
```

## User Space

Plan 9's user space is built around the shell (rc), window manager (rio), and integrated development environment (acme).

### User Environment

```mermaid
graph TB
    subgraph "Interactive Environment"
        RIO[Rio Window Manager]
        ACME[Acme Editor/IDE]
        RC[RC Shell]
        TERM[Terminal Emulator]
    end
    
    subgraph "Development Tools"
        CC[Plan 9 C Compiler]
        LD[Linker]
        MK[Mk Build System]
        ACID[Acid Debugger]
    end
    
    subgraph "System Services"
        PLUMBER[Plumber]
        FACTOTUM[Factotum]
        EXPORTFS[Exportfs]
    end
    
    RIO --> ACME
    RIO --> RC
    RIO --> TERM
    
    ACME --> CC
    ACME --> MK
    RC --> CC
    RC --> MK
    
    ACME --> PLUMBER
    RC --> FACTOTUM
    TERM --> EXPORTFS
```

### Plumber Architecture

```mermaid
graph LR
    subgraph "Source Applications"
        ACME[Acme]
        RIO[Rio]
        MAIL[Mail Reader]
    end
    
    subgraph "Plumber"
        RULES[Plumbing Rules]
        ROUTER[Message Router]
        PORTS[Plumber Ports]
    end
    
    subgraph "Destination Applications"
        BROWSER[Web Browser]
        EDITOR[Text Editor] 
        PLAYER[Media Player]
    end
    
    ACME --> RULES
    RIO --> RULES
    MAIL --> RULES
    
    RULES --> ROUTER
    ROUTER --> PORTS
    
    PORTS --> BROWSER
    PORTS --> EDITOR
    PORTS --> PLAYER
```

## Development Environment

Plan 9 provides a unique development environment centered around the mk build system and integrated tools.

### Build System Architecture

```mermaid
graph TB
    subgraph "Source Code"
        CSRC[C Source Files]
        HSRC[Header Files]
        MKFILE[Mkfiles]
    end
    
    subgraph "Build Process"
        MK[Mk Build System]
        CC[Plan 9 C Compiler]
        LD[Loader/Linker]
    end
    
    subgraph "Output"
        OBJ[Object Files]
        LIB[Libraries]
        EXEC[Executables]
    end
    
    MKFILE --> MK
    MK --> CC
    CSRC --> CC
    HSRC --> CC
    
    CC --> OBJ
    OBJ --> LD
    LD --> EXEC
    LD --> LIB
```

### Library Structure

```mermaid
graph TB
    subgraph "Core Libraries"
        LIBC[libc - C Library]
        LIB9P[lib9p - 9P Protocol]
        LIBTHREAD[libthread - Threading]
        LIBDRAW[libdraw - Graphics]
    end
    
    subgraph "System Libraries"
        LIBAUTH[libauth - Authentication]
        LIBSEC[libsec - Security/Crypto]
        LIBIP[libip - Networking]
        LIBDISK[libdisk - Disk Operations]
    end
    
    subgraph "Utility Libraries"
        LIBBIO[libbio - Buffered I/O]
        LIBREGEXP[libregexp - Regular Expressions]
        LIBHTML[libhtml - HTML Parsing]
        LIBMP[libmp - Multiple Precision Math]
    end
    
    LIBC --> LIBTHREAD
    LIBC --> LIBBIO
    LIB9P --> LIBAUTH
    LIBDRAW --> LIBTHREAD
    LIBAUTH --> LIBSEC
    LIBIP --> LIBC
```

---

*This documentation provides an overview of Plan 9's technical architecture. For detailed implementation information, refer to the papers in `/sys/doc/` and the manual pages in `/sys/man/`.*

## References

- `/sys/doc/9.ms` - Plan 9 from Bell Labs paper
- `/sys/doc/auth.ms` - Security in Plan 9
- `/sys/doc/prog4.ms` - Programming Environment Changes
- `/sys/doc/fossil.ms` - The Fossil File System
- `/sys/doc/venti/` - Venti documentation
- `/sys/man/` - Manual pages for all components