# Plan 9 Networking Architecture

## Overview

Plan 9's networking architecture is designed around the principle of network transparency, where network resources appear as files in the local namespace. The system supports multiple protocol stacks including its native IL protocol and standard TCP/IP.

## Network Stack Architecture

### Protocol Stack Overview

```mermaid
graph TB
    subgraph "Application Layer"
        NINEP[9P Protocol]
        HTTP[HTTP Services]
        SMTP[Mail Services]
        FTP[File Transfer]
        TELNET[Remote Terminal]
    end
    
    subgraph "Transport Layer"
        IL[IL - Internet Link]
        TCP[TCP - Transmission Control]
        UDP[UDP - User Datagram]
        RUDP[Reliable UDP]
    end
    
    subgraph "Network Layer"
        IP[IP - Internet Protocol]
        ICMP[ICMP - Control Messages]
        ARP[ARP - Address Resolution]
    end
    
    subgraph "Data Link Layer"
        ETHER[Ethernet]
        PPP[Point-to-Point]
        SLIP[Serial Line IP]
        LOOPBACK[Loopback]
    end
    
    NINEP --> IL
    NINEP --> TCP
    HTTP --> TCP
    SMTP --> TCP
    FTP --> TCP
    TELNET --> TCP
    
    IL --> IP
    TCP --> IP
    UDP --> IP
    RUDP --> IP
    
    IP --> ETHER
    IP --> PPP
    IP --> SLIP
    IP --> LOOPBACK
    
    ICMP --> IP
    ARP --> ETHER
```

## IL Protocol (Internet Link)

### IL Protocol Features

```mermaid
graph TB
    subgraph "IL Protocol Characteristics"
        RELIABLE[Reliable Delivery]
        ORDERED[Ordered Messages]
        FLOW[Flow Control]
        LIGHT[Lightweight]
    end
    
    subgraph "Advantages over TCP"
        SIMPLE[Simpler State Machine]
        FAST[Faster Connection Setup]
        EFFICIENT[More Efficient]
        NATIVE[Native to Plan 9]
    end
    
    RELIABLE --> SIMPLE
    ORDERED --> FAST
    FLOW --> EFFICIENT
    LIGHT --> NATIVE
```

### IL Message Format

```mermaid
classDiagram
    class ILHeader {
        +ushort sum
        +ushort len
        +uchar type
        +uchar spec
        +ushort src
        +ushort dst
        +ulong id
        +ulong ack
    }
    
    class ILTypes {
        +Sync : 0
        +Data : 1  
        +Dataquery : 2
        +Ack : 3
        +Query : 4
        +State : 5
        +Close : 6
        +CloseAck : 7
    }
    
    ILHeader --> ILTypes : type field
```

### IL State Machine

```mermaid
stateDiagram-v2
    [*] --> Closed
    Closed --> SyncSent: connect()
    SyncSent --> SyncRcvd: Sync received
    SyncRcvd --> Established: Ack sent/received
    SyncSent --> Established: SyncAck received
    
    Established --> DataXfer: send/receive data
    DataXfer --> Established: continue transfer
    
    Established --> CloseWait: Close received
    CloseWait --> Closed: CloseAck sent
    
    Established --> FinWait: close()
    FinWait --> Closed: CloseAck received
    
    Closed --> [*]
```

## Network File System Interface

### /net File System Structure

```mermaid
graph TB
    subgraph "/net File System"
        NET[/net]
        TCP[/net/tcp]
        UDP[/net/udp] 
        IL[/net/il]
        ETHER[/net/ether0]
        ARP[/net/arp]
    end
    
    subgraph "Connection Directories"
        TCPCONN[/net/tcp/0, /net/tcp/1, ...]
        UDPCONN[/net/udp/0, /net/udp/1, ...]
        ILCONN[/net/il/0, /net/il/1, ...]
    end
    
    subgraph "Connection Files"
        CTL[ctl - Control]
        DATA[data - Data transfer]
        LOCAL[local - Local address]
        REMOTE[remote - Remote address]
        STATUS[status - Connection status]
    end
    
    TCP --> TCPCONN
    UDP --> UDPCONN
    IL --> ILCONN
    
    TCPCONN --> CTL
    TCPCONN --> DATA
    TCPCONN --> LOCAL
    TCPCONN --> REMOTE
    TCPCONN --> STATUS
```

### Network Programming Model

```mermaid
sequenceDiagram
    participant App as Application
    participant Net as /net/tcp/clone
    participant Conn as /net/tcp/N
    participant Stack as TCP Stack
    
    App->>Net: open("/net/tcp/clone", ORDWR)
    Net->>App: return fd, allocate connection N
    
    App->>Conn: write("connect server.com!80")
    Conn->>Stack: initiate TCP connection
    Stack->>Conn: connection established
    
    App->>Conn: write(data) via data file
    Conn->>Stack: send TCP segments
    
    Stack->>Conn: receive TCP segments  
    Conn->>App: read(data) via data file
    
    App->>Conn: close connection
    Conn->>Stack: close TCP connection
```

## Network Configuration

### Network Database Structure

```mermaid
graph TB
    subgraph "Network Database Files"
        LOCAL[/lib/ndb/local]
        COMMON[/lib/ndb/common]
        HOSTS[/sys/lib/ndb/hosts]
    end
    
    subgraph "Database Entries"
        SYS[sys= entries]
        DOM[dom= entries]  
        IP[ip= entries]
        NET[ipnet= entries]
        AUTH[auth= entries]
    end
    
    subgraph "Services"
        CS[cs - Connection Server]
        DNS[dns - Name Server]
        DHCP[dhcp - DHCP Client]
    end
    
    LOCAL --> SYS
    LOCAL --> DOM
    LOCAL --> IP
    COMMON --> NET
    
    SYS --> CS
    DOM --> DNS
    IP --> CS
    NET --> DHCP
```

### Sample Network Configuration

```
# /lib/ndb/local
sys=auth dom=example.com ip=192.168.1.1 ether=00:1b:21:12:34:56
    auth=auth fs=fs cpu=cpu
    dns=192.168.1.1

sys=fs dom=example.com ip=192.168.1.2 ether=00:1b:21:12:34:57
    fs=fs

sys=cpu dom=example.com ip=192.168.1.3 ether=00:1b:21:12:34:58  
    cpu=cpu

ipnet=lan ip=192.168.1.0 ipmask=255.255.255.0
    ipgw=192.168.1.1
    dns=192.168.1.1
```

## Connection Server (cs)

### Connection Resolution

```mermaid
graph TB
    subgraph "Connection Server Process"
        REQUEST[Connection Request]
        PARSE[Parse Address]
        LOOKUP[Database Lookup]
        RESOLVE[Name Resolution]
        RETURN[Return Network Address]
    end
    
    subgraph "Address Types"
        SYMBOLIC[Symbolic: tcp!server!http]
        NUMERIC[Numeric: tcp!192.168.1.1!80]
        SERVICE[Service: tcp!*!80]
    end
    
    REQUEST --> PARSE
    PARSE --> SYMBOLIC
    PARSE --> NUMERIC
    PARSE --> SERVICE
    
    SYMBOLIC --> LOOKUP
    LOOKUP --> RESOLVE
    RESOLVE --> RETURN
    NUMERIC --> RETURN
    SERVICE --> RETURN
```

### cs Operation Flow

```mermaid
sequenceDiagram
    participant App as Application
    participant cs as Connection Server
    participant ndb as Network DB
    participant dns as DNS Server
    
    App->>cs: "tcp!server.com!http"
    cs->>ndb: lookup "server.com"
    
    alt Found in local DB
        ndb->>cs: return IP address
    else Not found locally
        cs->>dns: DNS query for server.com
        dns->>cs: return IP address
    end
    
    cs->>ndb: lookup service "http"
    ndb->>cs: return port 80
    
    cs->>App: "tcp!192.168.1.100!80"
```

## DNS Resolution

### DNS Architecture

```mermaid
graph TB
    subgraph "DNS Client"
        RESOLVER[DNS Resolver]
        CACHE[Local Cache]
        QUERY[Query Engine]
    end
    
    subgraph "DNS Servers"
        LOCAL_DNS[Local DNS Server]
        ROOT[Root Servers]
        TLD[TLD Servers]
        AUTH[Authoritative Servers]
    end
    
    subgraph "DNS Database"
        NDB[Network Database]
        HOSTS[Host Entries]
        REVERSE[Reverse Lookup]
    end
    
    RESOLVER --> CACHE
    CACHE --> QUERY
    QUERY --> LOCAL_DNS
    LOCAL_DNS --> ROOT
    ROOT --> TLD
    TLD --> AUTH
    
    LOCAL_DNS --> NDB
    NDB --> HOSTS
    NDB --> REVERSE
```

### DNS Query Process

```mermaid
sequenceDiagram
    participant Client
    participant LocalDNS
    participant RootDNS  
    participant TLD_DNS
    participant AuthDNS
    
    Client->>LocalDNS: Query server.example.com
    
    alt Cache hit
        LocalDNS->>Client: Return cached result
    else Cache miss
        LocalDNS->>RootDNS: Query .com
        RootDNS->>LocalDNS: Refer to .com TLD
        
        LocalDNS->>TLD_DNS: Query example.com
        TLD_DNS->>LocalDNS: Refer to example.com DNS
        
        LocalDNS->>AuthDNS: Query server.example.com
        AuthDNS->>LocalDNS: Return IP address
        
        LocalDNS->>Client: Return IP address
    end
```

## Network Services

### Core Network Services

```mermaid
graph TB
    subgraph "Essential Services"
        CS[cs - Connection Server]
        DNS[dns - Domain Name Service]
        DHCP[dhcp - Dynamic Host Config]
        NTP[ntp - Network Time Protocol]
    end
    
    subgraph "File Services"
        EXPORTFS[exportfs - Export File System]
        IMPORT[import - Import File System] 
        SRV[srv - Service Registry]
    end
    
    subgraph "Communication Services"
        MAIL[upas - Mail System]
        WEB[httpd - Web Server]
        FTP[ftpd - FTP Server]
        TELNET[telnetd - Telnet Server]
    end
    
    CS --> DNS
    DNS --> DHCP
    DHCP --> NTP
    
    EXPORTFS --> SRV
    IMPORT --> SRV
    
    MAIL --> CS
    WEB --> CS
    FTP --> CS
    TELNET --> CS
```

### Service Discovery

```mermaid
graph TB
    subgraph "Service Registry"
        SRV[/srv Directory]
        SERVICES[Service Files]
        ANNOUNCE[Announced Services]
    end
    
    subgraph "Service Types"
        FS[File Systems]
        AUTH[Authentication]
        PRINT[Print Services]
        CUSTOM[Custom Services]
    end
    
    SRV --> SERVICES
    SERVICES --> ANNOUNCE
    
    ANNOUNCE --> FS
    ANNOUNCE --> AUTH  
    ANNOUNCE --> PRINT
    ANNOUNCE --> CUSTOM
```

## Network Security

### Firewall and Filtering

```mermaid
graph TB
    subgraph "Network Security"
        FILTER[Packet Filtering]
        NAT[Network Address Translation]
        PROXY[Proxy Services]
        TUNNEL[Tunneling]
    end
    
    subgraph "Access Control"
        HOSTS[Host-based Security]
        USERS[User Authentication]
        SERVICES[Service Restrictions]
        PORTS[Port Access Control]
    end
    
    FILTER --> HOSTS
    NAT --> USERS
    PROXY --> SERVICES
    TUNNEL --> PORTS
```

### Network Authentication

```mermaid
sequenceDiagram
    participant Client
    participant Network
    participant AuthServer
    participant Service
    
    Client->>Network: Connect to service
    Network->>AuthServer: Authenticate client
    AuthServer->>Network: Return credentials
    Network->>Service: Forward authenticated request
    Service->>AuthServer: Verify credentials
    AuthServer->>Service: Confirm authentication
    Service->>Client: Provide service
```

## Performance and Optimization

### Network Performance Tuning

```mermaid
graph TB
    subgraph "Performance Factors"
        LATENCY[Network Latency]
        BANDWIDTH[Available Bandwidth]
        CONGESTION[Network Congestion]
        PROTOCOL[Protocol Overhead]
    end
    
    subgraph "Optimization Techniques"
        CACHING[Connection Caching]
        POOLING[Connection Pooling]
        COMPRESSION[Data Compression]
        BATCHING[Request Batching]
    end
    
    LATENCY --> CACHING
    BANDWIDTH --> COMPRESSION
    CONGESTION --> POOLING
    PROTOCOL --> BATCHING
```

### Network Monitoring

```mermaid
graph TB
    subgraph "Monitoring Tools"
        NETSTAT[netstat - Connection Status]
        PING[ping - Connectivity Test]
        TRACEROUTE[traceroute - Path Tracing]
        SNOOPY[snoopy - Packet Capture]
    end
    
    subgraph "Metrics"
        THROUGHPUT[Network Throughput]
        ERRORS[Error Rates]
        UTILIZATION[Link Utilization]
        RESPONSE[Response Times]
    end
    
    NETSTAT --> UTILIZATION
    PING --> RESPONSE
    TRACEROUTE --> ERRORS
    SNOOPY --> THROUGHPUT
```

## Distributed Computing Model

### Resource Sharing

```mermaid
graph TB
    subgraph "Distributed Resources"
        CPU[CPU Servers]
        STORAGE[File Servers]  
        AUTH[Authentication Servers]
        SERVICES[Application Servers]
    end
    
    subgraph "Access Methods"
        MOUNT[File System Mounting]
        IMPORT[Resource Import]
        EXPORT[Resource Export]
        PROXY[Service Proxy]
    end
    
    subgraph "Client Access"
        TERMINAL[Terminal Workstations]
        MOBILE[Mobile Clients]
        EMBEDDED[Embedded Systems]
    end
    
    CPU --> MOUNT
    STORAGE --> IMPORT
    AUTH --> EXPORT
    SERVICES --> PROXY
    
    MOUNT --> TERMINAL
    IMPORT --> MOBILE
    EXPORT --> EMBEDDED
    PROXY --> TERMINAL
```

### Load Balancing

```mermaid
graph TB
    subgraph "Load Balancing Strategies"
        ROUND_ROBIN[Round Robin]
        LEAST_CONN[Least Connections]
        RESOURCE[Resource Based]
        GEOGRAPHIC[Geographic]
    end
    
    subgraph "Implementation"
        DNS_LB[DNS-based]
        PROXY_LB[Proxy-based]
        CLIENT_LB[Client-based]
    end
    
    ROUND_ROBIN --> DNS_LB
    LEAST_CONN --> PROXY_LB
    RESOURCE --> CLIENT_LB
    GEOGRAPHIC --> DNS_LB
```

## Network Troubleshooting

### Diagnostic Workflow

```mermaid
flowchart TD
    PROBLEM[Network Problem] --> BASIC[Basic Connectivity]
    BASIC --> PING{ping works?}
    PING -->|No| PHYSICAL[Check Physical Layer]
    PING -->|Yes| DNS_CHECK[Check DNS Resolution]
    
    DNS_CHECK --> NSLOOKUP{nslookup works?}
    NSLOOKUP -->|No| DNS_FIX[Fix DNS Configuration]
    NSLOOKUP -->|Yes| SERVICE[Check Service]
    
    SERVICE --> CONNECT{Connection works?}
    CONNECT -->|No| PORT[Check Port/Firewall]
    CONNECT -->|Yes| AUTH[Check Authentication]
    
    PHYSICAL --> CABLE[Check Cables/Interface]
    DNS_FIX --> CONFIG[Update ndb files]
    PORT --> FIREWALL[Check Access Rules]
    AUTH --> CREDS[Verify Credentials]
```

### Common Network Commands

| Command | Purpose | Example |
|---------|---------|---------|
| `ping` | Test connectivity | `ping server.com` |
| `traceroute` | Trace network path | `traceroute server.com` |
| `netstat` | Show connections | `netstat -n` |
| `nslookup` | DNS lookup | `nslookup server.com` |
| `snoopy` | Packet capture | `snoopy -f tcp host server.com` |
| `cs` | Connection server | `cs` |
| `dns` | DNS server | `dns -r` |

## Integration with Plan 9

### Network Namespace Integration

```mermaid
graph TB
    subgraph "Process Namespace"
        ROOT[/]
        NET[/net]
        MOUNT[Mount Points]
        UNION[Union Directories]
    end
    
    subgraph "Network Resources"
        LOCAL[Local Network]
        REMOTE[Remote Networks]
        SERVICES[Network Services]
        DEVICES[Network Devices]
    end
    
    NET --> LOCAL
    MOUNT --> REMOTE
    UNION --> SERVICES
    ROOT --> DEVICES
```

### 9P over Network

```mermaid
sequenceDiagram
    participant Client
    participant Network
    participant Server
    participant FileSystem
    
    Client->>Network: 9P request over TCP/IL
    Network->>Server: Forward 9P message
    Server->>FileSystem: Process file operation
    FileSystem->>Server: Return result
    Server->>Network: 9P response
    Network->>Client: Forward response
    
    Note over Client,FileSystem: Transparent network file access
```

## References

- `/sys/doc/net/` - Network documentation
- `/sys/man/3/ip` - IP stack manual pages
- `/sys/man/8/dhcp` - DHCP configuration
- `/sys/src/9/ip/` - Network stack source code
- `/lib/ndb/` - Network database files
- `/sys/log/` - Network service logs