# Plan 9 Deployment and Configuration Guide

## System Architecture Overview

Plan 9 follows a distributed computing model where different services run on specialized servers connected via the 9P protocol.

### Typical Deployment Topology

```mermaid
graph TB
    subgraph "File Server"
        FS[File Server Process]
        FOSSIL[Fossil Active File System]
        VENTI[Venti Archival Storage]
        STORAGE[Physical Storage]
    end
    
    subgraph "Authentication Server"
        AUTH[Auth Server Process]
        KEYDB[Key Database]
        USERDB[User Database]
    end
    
    subgraph "CPU Server"
        CPUSRV[CPU Server Process]
        KERNEL[Plan 9 Kernel]
        SERVICES[System Services]
    end
    
    subgraph "Terminal Workstation"
        TERM[Terminal Process]
        RIO[Rio Window Manager]
        APPS[User Applications]
    end
    
    subgraph "Network"
        ETHER[Ethernet/Network]
    end
    
    FS --> ETHER
    AUTH --> ETHER
    CPUSRV --> ETHER
    TERM --> ETHER
    
    FS --> FOSSIL
    FOSSIL --> VENTI
    VENTI --> STORAGE
    AUTH --> KEYDB
    AUTH --> USERDB
    CPUSRV --> KERNEL
    KERNEL --> SERVICES
    TERM --> RIO
    RIO --> APPS
```

## Server Configuration

### File Server Setup

```mermaid
flowchart TD
    START[Start File Server Setup] --> VENTI_INIT[Initialize Venti Storage]
    VENTI_INIT --> VENTI_CONFIG[Configure Venti Arenas]
    VENTI_CONFIG --> FOSSIL_INIT[Initialize Fossil]
    FOSSIL_INIT --> FOSSIL_CONFIG[Configure Fossil File System]
    FOSSIL_CONFIG --> FS_START[Start File Server]
    FS_START --> FS_READY[File Server Ready]
    
    VENTI_CONFIG --> ARENA1[Arena 1]
    VENTI_CONFIG --> ARENA2[Arena 2]
    VENTI_CONFIG --> ARENAN[Arena N]
```

#### File Server Configuration Steps

1. **Venti Storage Setup**
   ```bash
   # Initialize venti storage
   venti/fmtarenas arenas /dev/sdC0/arenas
   venti/fmtisect isect /dev/sdC0/isect
   venti/fmtindex index /dev/sdC0/index isect
   ```

2. **Fossil File System Setup**
   ```bash
   # Initialize fossil
   fossil/flfmt -v /dev/sdC0/fossil
   # Start fossil with venti backend
   fossil/fossil -f /dev/sdC0/fossil -c 'srv fossil' -c 'srv fscons'
   ```

### Authentication Server Setup

```mermaid
sequenceDiagram
    participant Admin
    participant AuthServer
    participant KeyDB
    participant UserDB
    
    Admin->>AuthServer: Initialize auth database
    AuthServer->>KeyDB: Create key storage
    AuthServer->>UserDB: Create user database
    
    Admin->>AuthServer: Add hostowner
    Admin->>AuthServer: Configure network keys
    Admin->>AuthServer: Add user accounts
    
    AuthServer->>KeyDB: Store keys
    AuthServer->>UserDB: Store user info
```

#### Authentication Configuration

```bash
# Initialize authentication database
auth/keyfs -wp -m /mnt/keys /adm/keys
auth/changeuser -p glenda

# Add authentication server to network
echo 'auth=192.168.1.1' >> /lib/ndb/local

# Start authentication services  
auth/authsrv tcp!*!567
```

### CPU Server Configuration

```mermaid
graph TB
    subgraph "CPU Server Services"
        CPUSRV[CPU Service Listener]
        EXPORTFS[Export File System]
        FACTOTUM[Factotum Agent]
        LISTEN[Listen Service]
    end
    
    subgraph "Resource Management"
        PROC[Process Management]
        MEM[Memory Management]
        SCHED[Scheduler]
    end
    
    subgraph "Network Services"
        NINEP[9P Protocol Handler]
        IL[IL Protocol]
        TCP[TCP Stack]
    end
    
    CPUSRV --> PROC
    EXPORTFS --> NINEP
    FACTOTUM --> AUTH[Authentication]
    LISTEN --> TCP
    
    PROC --> SCHED
    PROC --> MEM
    NINEP --> IL
    NINEP --> TCP
```

## Network Configuration

### Network Database Configuration

```mermaid
graph TB
    subgraph "/lib/ndb/local"
        SYS[sys=server]
        DOM[dom=example.com]  
        IP[ip=192.168.1.1]
        AUTH[auth=192.168.1.1]
        FS[fs=192.168.1.2]
        CPU[cpu=192.168.1.3]
    end
    
    subgraph "Network Services"
        DNS[DNS Resolution]
        DHCP[DHCP Service]
        ROUTING[Routing Table]
    end
    
    SYS --> DNS
    DOM --> DNS
    IP --> ROUTING
    AUTH --> DNS
    FS --> DNS
    CPU --> DNS
```

### Sample Network Configuration

```
# /lib/ndb/local
sys=auth dom=example.com ip=192.168.1.1
    auth=auth
    
sys=fs dom=example.com ip=192.168.1.2
    fs=fs
    
sys=cpu dom=example.com ip=192.168.1.3  
    cpu=cpu
    
sys=term dom=example.com ip=192.168.1.100
    
ipnet=example ip=192.168.1.0 ipmask=255.255.255.0
    ipgw=192.168.1.1
    dns=192.168.1.1
```

## Service Management

### Boot Process Flow

```mermaid
sequenceDiagram
    participant Firmware
    participant Bootloader
    participant Kernel
    participant Init
    participant Services
    
    Firmware->>Bootloader: Power on
    Bootloader->>Kernel: Load kernel
    Kernel->>Init: Start init process
    Init->>Services: Start system services
    
    Note over Services: File server, auth server, etc.
    Services->>Services: Network configuration
    Services->>Services: Mount remote file systems
```

### Service Dependencies

```mermaid
graph TB
    subgraph "Boot Dependencies"
        KERNEL[Kernel] --> INIT[Init Process]
        INIT --> FOSSIL[Fossil FS]
        FOSSIL --> VENTI[Venti Storage]
        INIT --> NETWORK[Network Setup]
        NETWORK --> AUTHSRV[Auth Server]
        AUTHSRV --> CPUSRV[CPU Server]
        CPUSRV --> TERMINAL[Terminal Services]
    end
```

## User Environment Setup

### Terminal Configuration

```mermaid
graph TB
    subgraph "Terminal Startup"
        BOOT[Boot Terminal]
        DRAWTERM[Connect via drawterm]
        AUTH[Authenticate User]
        NAMESPACE[Setup Namespace]
        RIO[Start Rio]
        PROFILE[Load Profile]
    end
    
    BOOT --> AUTH
    DRAWTERM --> AUTH
    AUTH --> NAMESPACE
    NAMESPACE --> RIO
    RIO --> PROFILE
```

### Namespace Configuration

```mermaid
graph TB
    subgraph "Default Namespace"
        ROOT[/ - Root]
        BIN[/bin - Binaries]
        USR[/usr - User Files]
        TMP[/tmp - Temporary]
        PROC[/proc - Processes]
        NET[/net - Network]
        ENV[/env - Environment]
    end
    
    subgraph "User Namespace"
        HOME[/usr/$user - Home Directory]
        BIN_USER[/bin - User Binaries]
        LIB_USER[/lib - User Libraries]
    end
    
    ROOT --> BIN
    ROOT --> USR
    ROOT --> TMP
    ROOT --> PROC
    ROOT --> NET
    ROOT --> ENV
    USR --> HOME
    HOME --> BIN_USER
    HOME --> LIB_USER
```

## Storage Configuration

### Fossil + Venti Architecture

```mermaid
graph TB
    subgraph "Active Storage (Fossil)"
        ACTIVE[Active File System]
        CACHE[Block Cache]
        TEMP[Temporary Blocks]
    end
    
    subgraph "Archival Storage (Venti)"
        BLOCKS[Content-Addressed Blocks]
        ARENAS[Storage Arenas]
        INDEX[Block Index]
    end
    
    subgraph "Physical Storage"
        DISK1[Disk 1 - Fossil]
        DISK2[Disk 2 - Venti Arenas]
        DISK3[Disk 3 - Venti Index]
    end
    
    ACTIVE --> CACHE
    CACHE --> TEMP
    ACTIVE -->|Daily Snapshot| BLOCKS
    BLOCKS --> ARENAS
    BLOCKS --> INDEX
    
    ACTIVE --> DISK1
    ARENAS --> DISK2
    INDEX --> DISK3
```

### Storage Partitioning

```mermaid
graph TB
    subgraph "Disk Layout"
        MBR[Master Boot Record]
        P9PART[Plan 9 Partition Table]
        FOSSIL_PART[Fossil Partition]  
        VENTI_PART[Venti Partition]
        SWAP_PART[Swap Partition]
    end
    
    MBR --> P9PART
    P9PART --> FOSSIL_PART
    P9PART --> VENTI_PART  
    P9PART --> SWAP_PART
```

## Security Configuration

### Authentication Flow

```mermaid
sequenceDiagram
    participant User
    participant Terminal
    participant AuthServer
    participant CPUServer
    participant FileServer
    
    User->>Terminal: Login request
    Terminal->>AuthServer: Authenticate user
    AuthServer->>Terminal: Return credentials
    Terminal->>CPUServer: Connect with credentials
    CPUServer->>FileServer: Access files with user identity
    FileServer->>AuthServer: Validate access
    AuthServer->>FileServer: Confirm permissions
    FileServer->>CPUServer: Provide file access
```

### Factotum Configuration

```mermaid
graph TB
    subgraph "Factotum Agent"
        KEYS[Key Storage]
        PROTO[Protocol Handlers]
        SECSTORE[Secstore Client]
    end
    
    subgraph "Authentication Protocols"
        P9SK1[p9sk1 - Plan 9 shared key]
        P9SK2[p9sk2 - Plan 9 public key]
        APOP[apop - POP3 authentication]
        CRAM[cram - CRAM-MD5]
    end
    
    KEYS --> PROTO
    PROTO --> P9SK1
    PROTO --> P9SK2
    PROTO --> APOP
    PROTO --> CRAM
    SECSTORE --> KEYS
```

## Monitoring and Maintenance

### System Monitoring

```mermaid
graph TB
    subgraph "Monitoring Tools"
        STATS[/proc/stats]
        PS[ps - Process list]
        WHO[who - Active users]
        NETSTAT[netstat - Network status]
    end
    
    subgraph "Log Files"
        FOSSIL_LOG[Fossil console log]
        VENTI_LOG[Venti log]
        AUTH_LOG[Authentication log]
        SYSTEM_LOG[System console]
    end
    
    STATS --> SYSTEM_LOG
    PS --> SYSTEM_LOG
    WHO --> AUTH_LOG
    NETSTAT --> SYSTEM_LOG
```

### Backup Strategy

```mermaid
graph TB
    subgraph "Backup Process"
        DAILY[Daily Snapshot]
        WEEKLY[Weekly Archive]
        MONTHLY[Monthly Backup]
    end
    
    subgraph "Storage Tiers"
        FOSSIL[Fossil Active]
        VENTI[Venti Archive] 
        TAPE[Tape Backup]
        OFFSITE[Offsite Storage]
    end
    
    DAILY --> FOSSIL
    FOSSIL --> VENTI
    WEEKLY --> VENTI
    VENTI --> TAPE
    MONTHLY --> TAPE
    TAPE --> OFFSITE
```

## Troubleshooting

### Common Issues

```mermaid
flowchart TD
    PROBLEM[System Problem] --> NET_CHECK{Network OK?}
    NET_CHECK -->|No| NET_FIX[Fix Network]
    NET_CHECK -->|Yes| AUTH_CHECK{Auth OK?}
    AUTH_CHECK -->|No| AUTH_FIX[Fix Authentication]
    AUTH_CHECK -->|Yes| FS_CHECK{File System OK?}
    FS_CHECK -->|No| FS_FIX[Fix File System]
    FS_CHECK -->|Yes| APP_CHECK[Check Application]
    
    NET_FIX --> RESTART[Restart Services]
    AUTH_FIX --> RESTART
    FS_FIX --> RESTART
    APP_CHECK --> RESTART
    RESTART --> VERIFY[Verify Operation]
```

### Diagnostic Commands

| Issue | Command | Description |
|-------|---------|-------------|
| Network | `ip/ping host` | Test connectivity |
| Authentication | `auth/debug` | Debug auth issues |
| File System | `fossil/conf` | Check fossil status |
| Processes | `ps -a` | List all processes |
| Memory | `cat /proc/swap` | Check swap usage |

## References

- `/sys/doc/` - System documentation
- `/sys/man/8/` - System administration manual pages  
- `/sys/man/4/` - Device and file system manual pages
- `/lib/namespace*` - Namespace configuration files
- `/rc/bin/cpurc` - CPU server startup script
- `/rc/bin/termrc` - Terminal startup script