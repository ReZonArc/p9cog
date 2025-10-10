# Plan 9 File System Architecture

## Overview

Plan 9's file system architecture consists of two main components: Fossil for active storage and Venti for archival storage. This design provides both high-performance access to current data and reliable long-term storage.

## Fossil File System

### Architecture Overview

```mermaid
graph TB
    subgraph "Fossil File System"
        CACHE[Block Cache]
        ACTIVE[Active Blocks]
        SNAP[Snapshot Blocks]
        TEMP[Temporary Blocks]
    end
    
    subgraph "Venti Backend"
        ARCHIVE[Archived Blocks]
        SCORES[SHA1 Scores]
    end
    
    subgraph "Disk Storage"
        DISK[Raw Disk Blocks]
    end
    
    CACHE --> ACTIVE
    ACTIVE --> TEMP
    ACTIVE --> SNAP
    SNAP -->|Daily Snapshot| ARCHIVE
    ARCHIVE --> SCORES
    
    CACHE --> DISK
    TEMP --> DISK
```

### Block Types

```mermaid
classDiagram
    class Block {
        +uchar type
        +uint addr
        +uint tag
        +uchar data[...]
    }
    
    class RootBlock {
        +char name[128]
        +char type[128]
        +uchar score[VtScoreSize]
        +uint blocksize
        +uchar prev[VtScoreSize]
    }
    
    class SuperBlock {
        +uint version
        +uint epoch
        +uchar qid
        +uchar active[VtScoreSize]
        +uchar next[VtScoreSize]
        +uchar current[VtScoreSize]
    }
    
    class DirBlock {
        +DirEntry entries[]
    }
    
    class DataBlock {
        +uchar data[blocksize]
    }
    
    class PointerBlock {
        +uchar ptrs[blocksize/VtScoreSize][VtScoreSize]
    }
    
    Block <|-- RootBlock
    Block <|-- SuperBlock  
    Block <|-- DirBlock
    Block <|-- DataBlock
    Block <|-- PointerBlock
```

### File Structure

```mermaid
graph TB
    subgraph "File Representation"
        ROOT[Root Block]
        META[Meta Block]
        PTR1[Pointer Block L1]
        PTR2[Pointer Block L2] 
        DATA1[Data Block 1]
        DATA2[Data Block 2]
        DATA3[Data Block 3]
        DATA4[Data Block 4]
    end
    
    ROOT --> META
    META --> PTR1
    PTR1 --> PTR2
    PTR1 --> DATA1
    PTR2 --> DATA2
    PTR2 --> DATA3
    PTR2 --> DATA4
```

### Directory Structure

```mermaid
classDiagram
    class DirEntry {
        +char elem[256]
        +uint version
        +uint qid
        +uchar score[VtScoreSize]
        +uchar mscore[VtScoreSize]
        +uvlong size
        +uint mode
        +uint uid
        +uint gid
        +uint mtime
        +uint atime
    }
    
    class MetaBlock {
        +uint magic
        +uint version
        +uint size
        +uint free
        +uint maxindex
        +uint nindex
        +DirEntry entries[]
    }
    
    MetaBlock --> DirEntry : contains
```

## Venti Archive Storage

### Content-Addressed Storage

```mermaid
graph TB
    subgraph "Venti Server"
        WRITE[Write Request]
        HASH[SHA1 Hash]
        LOOKUP[Score Lookup]
        STORE[Block Storage]
        READ[Read Request]
    end
    
    subgraph "Storage Structure"
        INDEX[Index Blocks]
        ARENAS[Storage Arenas]
        LOG[Write Log]
    end
    
    WRITE --> HASH
    HASH --> LOOKUP
    LOOKUP -->|Not Found| STORE
    LOOKUP -->|Found| DUPLICATE[Duplicate - Return Score]
    STORE --> ARENAS
    STORE --> LOG
    STORE --> INDEX
    
    READ --> INDEX
    INDEX --> ARENAS
```

### Block Types in Venti

```mermaid
classDiagram
    class VtBlock {
        +uchar type
        +uchar score[VtScoreSize]  
        +uint size
        +uchar data[]
    }
    
    class VtDataType {
        +VtDataType : 0
        +VtDirType : 1
        +VtRootType : 2
    }
    
    class VtPointerType {
        +VtPointerType0 : 3
        +VtPointerType1 : 4 
        +VtPointerType2 : 5
        +VtPointerType3 : 6
        +VtPointerType4 : 7
        +VtPointerType5 : 8
        +VtPointerType6 : 9
    }
    
    VtBlock --> VtDataType : data blocks
    VtBlock --> VtPointerType : pointer blocks
```

### Arena Structure

```mermaid
graph TB
    subgraph "Arena Layout"
        HEADER[Arena Header]
        BLOCKS[Data Blocks]
        TRAILER[Arena Trailer]
    end
    
    subgraph "Arena Header"
        MAGIC[Magic Number]
        VERSION[Version]
        NAME[Arena Name]
        BLOCKSIZE[Block Size]
        BASE[Base Address]
    end
    
    subgraph "Arena Trailer" 
        INDEX[Index Data]
        SEAL[Seal/Checksum]
    end
    
    HEADER --> MAGIC
    HEADER --> VERSION  
    HEADER --> NAME
    HEADER --> BLOCKSIZE
    HEADER --> BASE
    
    TRAILER --> INDEX
    TRAILER --> SEAL
    
    HEADER --> BLOCKS
    BLOCKS --> TRAILER
```

## File Operations

### Read Operation Flow

```mermaid
sequenceDiagram
    participant Client
    participant Fossil
    participant Cache
    participant Venti
    
    Client->>Fossil: Read file block
    Fossil->>Cache: Check block cache
    
    alt Block in cache
        Cache->>Fossil: Return cached block
        Fossil->>Client: Return data
    else Block not in cache
        Fossil->>Fossil: Check if block is active
        
        alt Active block
            Fossil->>Cache: Read from disk
            Cache->>Fossil: Return block
        else Archived block  
            Fossil->>Venti: Request by score
            Venti->>Fossil: Return block
            Fossil->>Cache: Cache block
        end
        
        Fossil->>Client: Return data
    end
```

### Write Operation Flow

```mermaid
sequenceDiagram
    participant Client
    participant Fossil
    participant Cache
    participant Venti
    
    Client->>Fossil: Write data
    Fossil->>Cache: Allocate new block
    Cache->>Fossil: Return block address
    
    Fossil->>Fossil: Update metadata
    Fossil->>Cache: Mark blocks dirty
    
    Note over Fossil,Cache: Blocks written to disk periodically
    
    opt Daily snapshot
        Fossil->>Venti: Archive old blocks
        Venti->>Fossil: Return scores
        Fossil->>Fossil: Update pointers to scores
    end
    
    Fossil->>Client: Write complete
```

## Snapshots and Versioning

### Snapshot Mechanism

```mermaid
graph TB
    subgraph "Active File System"
        CURRENT[Current Root]
        ACTIVE_BLOCKS[Active Blocks]
    end
    
    subgraph "Snapshot Process"
        SNAP_ROOT[Snapshot Root]
        COPY_BLOCKS[Copy-on-Write Blocks]
        VENTI_ARCHIVE[Archive to Venti]
    end
    
    subgraph "Historical Versions"
        SNAP1[Snapshot Day 1]
        SNAP2[Snapshot Day 2]
        SNAP3[Snapshot Day 3]
    end
    
    CURRENT --> SNAP_ROOT
    ACTIVE_BLOCKS --> COPY_BLOCKS
    COPY_BLOCKS --> VENTI_ARCHIVE
    VENTI_ARCHIVE --> SNAP1
    VENTI_ARCHIVE --> SNAP2
    VENTI_ARCHIVE --> SNAP3
```

### Copy-on-Write

```mermaid
sequenceDiagram
    participant Process
    participant Fossil
    participant Block
    participant NewBlock
    
    Process->>Fossil: Modify shared block
    Fossil->>Block: Check reference count
    
    alt Single reference
        Fossil->>Block: Modify in place
        Block->>Process: Modified block
    else Multiple references
        Fossil->>NewBlock: Allocate new block
        Fossil->>NewBlock: Copy data from old block
        Fossil->>NewBlock: Apply modifications
        Fossil->>Fossil: Update pointer to new block
        NewBlock->>Process: Modified block
    end
```

## Performance Optimization

### Caching Strategy

```mermaid
graph TB
    subgraph "Fossil Cache Hierarchy"
        L1[Block Cache - Memory]
        L2[Disk Cache - SSD]
        L3[Active Storage - HDD]
        L4[Archive Storage - Venti]
    end
    
    subgraph "Cache Policies"
        LRU[LRU Eviction]
        DIRTY[Dirty Block Tracking]
        PREFETCH[Prefetch Heuristics]
    end
    
    L1 --> L2
    L2 --> L3
    L3 --> L4
    
    L1 --> LRU
    L1 --> DIRTY
    L1 --> PREFETCH
```

### I/O Optimization

```mermaid
graph TB
    subgraph "Read Optimization"
        READAHEAD[Read-ahead]
        CLUSTERING[Block Clustering]
        PARALLEL[Parallel I/O]
    end
    
    subgraph "Write Optimization"  
        DELAYED[Delayed Write]
        BATCHING[Write Batching]
        ORDERING[Write Ordering]
    end
    
    READAHEAD --> CLUSTERING
    CLUSTERING --> PARALLEL
    
    DELAYED --> BATCHING
    BATCHING --> ORDERING
```

## File System Consistency

### Consistency Model

```mermaid
graph TB
    subgraph "Consistency Guarantees"
        ATOMIC[Atomic Updates]
        ORDERING[Write Ordering] 
        DURABILITY[Durability]
        RECOVERY[Crash Recovery]
    end
    
    subgraph "Implementation"
        EPOCHS[Epoch System]
        LOGGING[Write-Ahead Logging]
        CHECKSUM[Block Checksums]
        REPLAY[Log Replay]
    end
    
    ATOMIC --> EPOCHS
    ORDERING --> LOGGING
    DURABILITY --> CHECKSUM
    RECOVERY --> REPLAY
```

### Recovery Process

```mermaid
sequenceDiagram
    participant System
    participant Fossil
    participant Log
    participant Venti
    
    Note over System: System crash/restart
    
    System->>Fossil: Mount file system
    Fossil->>Log: Read write log
    
    loop For each log entry
        Log->>Fossil: Replay operation
        Fossil->>Fossil: Apply changes
    end
    
    Fossil->>Venti: Verify archived blocks
    Venti->>Fossil: Confirm integrity
    
    Fossil->>System: File system ready
```

## Tools and Utilities

### File System Tools

```mermaid
graph TB
    subgraph "Fossil Tools"
        FLCHK[flchk - Check consistency]
        FLFMT[flfmt - Format file system] 
        CONF[conf - Configuration]
        SNAP[snap - Snapshot management]
    end
    
    subgraph "Venti Tools"
        VTCHK[venti/check - Verify integrity]
        VTFMT[venti/fmt* - Format components]
        VTREAD[venti/read - Read blocks]
        VTWRITE[venti/write - Write blocks]
    end
    
    subgraph "Debug Tools"
        VTDUMP[venti/dump - Dump blocks]
        FLDUMP[fossil/dump - Dump structures]
        FSCK[fsck - File system check]
    end
```

### Administrative Commands

| Tool | Purpose | Usage |
|------|---------|-------|
| `fossil/conf` | Configure fossil | `fossil/conf /dev/fscons` |
| `fossil/flchk` | Check consistency | `flchk /dev/fossil` |
| `venti/buildindex` | Rebuild index | `buildindex venti.conf` |
| `venti/checkindex` | Verify index | `checkindex venti.conf` |
| `fossil/snap` | Manage snapshots | `snap -a archive` |

## Integration with Plan 9

### File Server Integration

```mermaid
graph TB
    subgraph "Plan 9 Integration"
        KERNEL[Plan 9 Kernel]
        NINEP[9P Protocol]
        MOUNT[Mount Points]
        NAMESPACE[Namespace]
    end
    
    subgraph "File Server"
        FS[File Server Process]  
        FOSSIL[Fossil FS]
        VENTI[Venti Server]
    end
    
    KERNEL --> NINEP
    NINEP --> MOUNT
    MOUNT --> NAMESPACE
    NAMESPACE --> FS
    FS --> FOSSIL
    FOSSIL --> VENTI
```

### Network File Access

```mermaid
sequenceDiagram
    participant Terminal
    participant CPU_Server  
    participant File_Server
    participant Fossil
    participant Venti
    
    Terminal->>CPU_Server: 9P file request
    CPU_Server->>File_Server: Forward request
    File_Server->>Fossil: File system operation
    
    alt Recent data
        Fossil->>File_Server: Return from active storage
    else Archived data
        Fossil->>Venti: Retrieve archived block
        Venti->>Fossil: Return block
        Fossil->>File_Server: Return data
    end
    
    File_Server->>CPU_Server: 9P response
    CPU_Server->>Terminal: Forward response
```

## References

- `/sys/doc/fossil.ms` - Fossil File System paper
- `/sys/doc/venti/` - Venti documentation  
- `/sys/man/4/fossil` - Fossil manual page
- `/sys/man/8/venti` - Venti manual pages
- `/sys/src/cmd/fossil/` - Fossil source code
- `/sys/src/cmd/venti/` - Venti source code