# 9P Protocol Architecture

## Overview

The 9P protocol is Plan 9's distributed file protocol that enables transparent access to resources across the network. It treats all resources as files in a hierarchical namespace.

## Protocol Overview

### 9P Message Types

```mermaid
graph TB
    subgraph "Message Pairs"
        TVERSION[Tversion] --> RVERSION[Rversion]
        TAUTH[Tauth] --> RAUTH[Rauth] 
        TATTACH[Tattach] --> RATTACH[Rattach]
        TWALK[Twalk] --> RWALK[Rwalk]
        TOPEN[Topen] --> ROPEN[Ropen]
        TCREATE[Tcreate] --> RCREATE[Rcreate]
        TREAD[Tread] --> RREAD[Rread]
        TWRITE[Twrite] --> RWRITE[Rwrite]
        TCLUNK[Tclunk] --> RCLUNK[Rclunk]
        TREMOVE[Tremove] --> RREMOVE[Rremove]
        TSTAT[Tstat] --> RSTAT[Rstat]
        TWSTAT[Twstat] --> RWSTAT[Rwstat]
    end
    
    subgraph "Error Handling"
        ERROR[Rerror - Error Response]
    end
    
    TVERSION -.->|"on error"| ERROR
    TAUTH -.->|"on error"| ERROR
    TATTACH -.->|"on error"| ERROR
    TWALK -.->|"on error"| ERROR
    TOPEN -.->|"on error"| ERROR
    TCREATE -.->|"on error"| ERROR
    TREAD -.->|"on error"| ERROR
    TWRITE -.->|"on error"| ERROR
    TCLUNK -.->|"on error"| ERROR
    TREMOVE -.->|"on error"| ERROR
    TSTAT -.->|"on error"| ERROR
    TWSTAT -.->|"on error"| ERROR
```

## Connection Establishment

### Protocol Handshake

```mermaid
sequenceDiagram
    participant Client
    participant Server
    
    Note over Client,Server: Version Negotiation
    Client->>Server: Tversion(msize, version)
    Server->>Client: Rversion(msize, version)
    
    Note over Client,Server: Authentication (Optional)
    Client->>Server: Tauth(afid, uname, aname)
    Server->>Client: Rauth(aqid) / Rerror
    
    Note over Client,Server: File System Attachment
    Client->>Server: Tattach(fid, afid, uname, aname)
    Server->>Client: Rattach(qid)
    
    Note over Client,Server: Ready for File Operations
```

### FID Management

```mermaid
stateDiagram-v2
    [*] --> Unused: fid available
    Unused --> Attached: Tattach success
    Attached --> Walked: Twalk success
    Walked --> Opened: Topen success
    Opened --> Reading: Tread
    Opened --> Writing: Twrite
    Reading --> Opened: Rread
    Writing --> Opened: Rwrite
    
    Attached --> Unused: Tclunk
    Walked --> Unused: Tclunk
    Opened --> Unused: Tclunk
    
    Walked --> Removed: Tremove
    Removed --> [*]
```

## File Operations

### Directory Walking

```mermaid
sequenceDiagram
    participant Client
    participant Server
    
    Note over Client,Server: Navigate to /usr/rob/hello.c
    Client->>Server: Twalk(fid=1, newfid=2, ["usr"])
    Server->>Client: Rwalk([usr_qid])
    
    Client->>Server: Twalk(fid=2, newfid=3, ["rob"])  
    Server->>Client: Rwalk([rob_qid])
    
    Client->>Server: Twalk(fid=3, newfid=4, ["hello.c"])
    Server->>Client: Rwalk([hello_qid])
    
    Note over Client,Server: File Located at fid=4
```

### File I/O Operations

```mermaid
sequenceDiagram
    participant Client
    participant Server
    
    Note over Client,Server: Open File for Reading
    Client->>Server: Topen(fid=4, mode=OREAD)
    Server->>Client: Ropen(qid, iounit)
    
    Note over Client,Server: Read File Contents
    Client->>Server: Tread(fid=4, offset=0, count=8192)
    Server->>Client: Rread(data[0..n])
    
    Client->>Server: Tread(fid=4, offset=n, count=8192) 
    Server->>Client: Rread(data[n..m])
    
    Note over Client,Server: Continue until EOF (count=0)
    
    Client->>Server: Tclunk(fid=4)
    Server->>Client: Rclunk()
```

## Message Structure

### 9P Message Format

```mermaid
graph TB
    subgraph "Message Header"
        SIZE[Size - 4 bytes]
        TYPE[Type - 1 byte]  
        TAG[Tag - 2 bytes]
    end
    
    subgraph "Message Body"
        DATA[Message-specific data]
        STRINGS[Variable-length strings]
        QIDS[Qid structures]
        STATS[Stat structures]
    end
    
    SIZE --> TYPE
    TYPE --> TAG
    TAG --> DATA
    DATA --> STRINGS
    DATA --> QIDS  
    DATA --> STATS
```

### QID Structure

```mermaid
classDiagram
    class Qid {
        +uchar type
        +ulong vers
        +uvlong path
    }
    
    class QidType {
        +QTDIR : 0x80
        +QTAPPEND : 0x40
        +QTEXCL : 0x20
        +QTMOUNT : 0x10
        +QTAUTH : 0x08
        +QTTMP : 0x04
        +QTFILE : 0x00
    }
    
    Qid --> QidType : type field
```

### Stat Structure

```mermaid
classDiagram
    class Stat {
        +ushort type
        +ulong dev
        +Qid qid
        +ulong mode
        +ulong atime
        +ulong mtime
        +uvlong length
        +char *name
        +char *uid
        +char *gid
        +char *muid
    }
    
    class Mode {
        +DMDIR : 0x80000000
        +DMAPPEND : 0x40000000  
        +DMEXCL : 0x20000000
        +DMTMP : 0x04000000
        +DMREAD : 0x4
        +DMWRITE : 0x2
        +DMEXEC : 0x1
    }
    
    Stat --> Mode : mode field
```

## Protocol Variants

### 9P Versions

```mermaid
graph TB
    subgraph "Protocol Evolution"
        P9P1[9P Original] --> P9P2000[9P2000]
        P9P2000 --> P9P2000U[9P2000.u]
        P9P2000U --> P9P2000L[9P2000.L]
    end
    
    subgraph "Features"
        P9P1 --> F1[Basic file operations]
        P9P2000 --> F2[Authentication protocol]
        P9P2000U --> F3[Unicode support]
        P9P2000L --> F4[Linux extensions]
    end
```

### Transport Protocols

```mermaid
graph TB
    subgraph "9P Transport Options"
        TCP[TCP/IP]
        IL[IL Protocol]
        PIPE[Named Pipes]
        UNIX[Unix Domain Sockets]
        VIRTIO[VirtIO 9P]
    end
    
    subgraph "Network Stack"
        NINEP[9P Protocol Layer]
    end
    
    TCP --> NINEP
    IL --> NINEP
    PIPE --> NINEP  
    UNIX --> NINEP
    VIRTIO --> NINEP
```

## Security Model

### Authentication Flow

```mermaid
sequenceDiagram
    participant Client
    participant AuthServer
    participant FileServer
    
    Note over Client,FileServer: Establish Auth Channel
    Client->>FileServer: Tauth(afid, uname, aname)
    FileServer->>Client: Rauth(aqid)
    
    Note over Client,AuthServer: Authenticate with Auth Server
    Client->>AuthServer: Challenge/Response Protocol
    AuthServer->>Client: Ticket + Authenticator
    
    Note over Client,FileServer: Present Credentials
    Client->>FileServer: Twrite(afid, ticket+authenticator)
    FileServer->>Client: Rwrite(count)
    
    Note over Client,FileServer: Attach with Auth
    Client->>FileServer: Tattach(fid, afid, uname, aname)
    FileServer->>Client: Rattach(qid)
```

### Permission Model

```mermaid
graph TB
    subgraph "File Permissions"
        OWNER[Owner rwx]
        GROUP[Group rwx] 
        OTHER[Other rwx]
        SPECIAL[Special Bits]
    end
    
    subgraph "Directory Permissions"  
        DIR_R[Read - list contents]
        DIR_W[Write - create/remove files]
        DIR_X[Execute - search/traverse]
    end
    
    subgraph "9P Access Control"
        USER[User Identity]
        GROUPS[Group Membership]
        MODE[File Mode Bits]
        ACL[Access Control via /adm/users]
    end
    
    USER --> MODE
    GROUPS --> MODE
    MODE --> OWNER
    MODE --> GROUP  
    MODE --> OTHER
    MODE --> SPECIAL
```

## Implementation Architecture

### Server-Side Components

```mermaid
graph TB
    subgraph "9P Server"
        LISTEN[Connection Listener]
        MUXER[Request Multiplexer]
        WORKER[Worker Threads]
        FS[File System Backend]
    end
    
    subgraph "Client Connections"
        CONN1[Client Connection 1]
        CONN2[Client Connection 2]
        CONN3[Client Connection N]
    end
    
    CONN1 --> LISTEN
    CONN2 --> LISTEN
    CONN3 --> LISTEN
    LISTEN --> MUXER
    MUXER --> WORKER
    WORKER --> FS
```

### Client-Side Components

```mermaid
graph TB
    subgraph "9P Client"
        MOUNT[Mount Interface]
        CACHE[Attribute Cache]
        CONN[Connection Manager]
        PROTO[Protocol Handler]
    end
    
    subgraph "Applications"
        APP1[Application 1]
        APP2[Application 2]
        SHELL[Shell]
    end
    
    APP1 --> MOUNT
    APP2 --> MOUNT
    SHELL --> MOUNT
    MOUNT --> CACHE
    CACHE --> CONN
    CONN --> PROTO
```

## Performance Considerations

### Message Batching

```mermaid
sequenceDiagram
    participant Client
    participant Server
    
    Note over Client,Server: Efficient Directory Listing
    Client->>Server: Twalk(fid, newfid, ["dir"])
    Server->>Client: Rwalk([dir_qid])
    
    Client->>Server: Topen(newfid, OREAD)  
    Server->>Client: Ropen(qid, iounit)
    
    loop Read Directory Entries
        Client->>Server: Tread(newfid, offset, count)
        Server->>Client: Rread(stat_entries)
    end
    
    Client->>Server: Tclunk(newfid)
    Server->>Client: Rclunk()
```

### Caching Strategy

```mermaid
graph TB
    subgraph "Client-Side Caching"
        ATTR_CACHE[Attribute Cache]
        DATA_CACHE[Data Cache] 
        DENTRY_CACHE[Directory Entry Cache]
    end
    
    subgraph "Cache Policies"
        TTL[Time-based expiration]
        VALIDATE[Server validation]
        COHERENCE[Cache coherence protocol]
    end
    
    ATTR_CACHE --> TTL
    DATA_CACHE --> VALIDATE
    DENTRY_CACHE --> COHERENCE
```

## References

- `/sys/man/5/0intro` - Introduction to file servers
- `/sys/man/5/attach` - 9P attach protocol  
- `/sys/doc/9.ms` - Plan 9 from Bell Labs (9P description)
- `/sys/src/lib9p/` - 9P library implementation
- `RFC` - 9P2000 Protocol Specification