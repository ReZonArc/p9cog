# Plan 9 Development Environment

## Overview

Plan 9 provides an integrated development environment centered around the Acme editor, mk build system, and a collection of programming tools designed for simplicity and composability.

## Development Tools Overview

```mermaid
graph TB
    subgraph "Editors & IDE"
        ACME[Acme - Integrated Editor]
        SAM[Sam - Structural Editor]
        ED[Ed - Line Editor]
    end
    
    subgraph "Build System"
        MK[Mk - Build Tool]
        MKFILE[Mkfiles - Build Scripts]
        DEPS[Dependency Management]
    end
    
    subgraph "Compilers"
        CC[Plan 9 C Compiler]
        CPP[C Preprocessor]
        LD[Linker/Loader]
        AR[Archive Tool]
    end
    
    subgraph "Debug Tools"
        ACID[Acid Debugger]
        DB[Db Debugger]
        PROF[Profiling Tools]
    end
    
    ACME --> MK
    MK --> CC
    CC --> LD
    ACID --> CC
    SAM --> MKFILE
```

## Acme Editor

### Architecture

```mermaid
graph TB
    subgraph "Acme Components"
        COLS[Columns]
        WINS[Windows]
        TAGS[Tag Lines]
        BODY[Body Text]
        SCROLL[Scroll Bars]
    end
    
    subgraph "Integration"
        PLUMBER[Plumber Service]
        EXEC[Command Execution]
        FS[File System Access]
        MOUSE[Mouse Interface]
    end
    
    COLS --> WINS
    WINS --> TAGS
    WINS --> BODY
    WINS --> SCROLL
    
    TAGS --> EXEC
    EXEC --> PLUMBER
    PLUMBER --> FS
    MOUSE --> EXEC
```

### Window Management

```mermaid
stateDiagram-v2
    [*] --> Column: New column
    Column --> Window: Add window
    Window --> Dirty: Edit text
    Dirty --> Clean: Put (save)
    Clean --> Dirty: Modify text
    Window --> Closed: Del command
    Closed --> [*]
    
    Window --> Executing: Execute command
    Executing --> Window: Command complete
```

### Command Integration

```mermaid
sequenceDiagram
    participant User
    participant Acme
    participant Plumber
    participant Command
    
    User->>Acme: Right-click on text
    Acme->>Plumber: Send message
    Plumber->>Command: Route to appropriate tool
    Command->>Acme: Update display/open file
    
    Note over User,Command: Example: Click filename opens editor
```

## Build System (Mk)

### Mkfile Structure

```mermaid
graph TB
    subgraph "Mkfile Components"
        VARS[Variables]
        RULES[Build Rules]
        TARGETS[Targets]
        DEPS[Dependencies]
    end
    
    subgraph "Rule Processing"
        PATTERN[Pattern Rules]
        IMPLICIT[Implicit Rules]
        EXPLICIT[Explicit Rules]
        COMMANDS[Shell Commands]
    end
    
    VARS --> RULES
    RULES --> TARGETS
    TARGETS --> DEPS
    
    RULES --> PATTERN
    RULES --> IMPLICIT
    RULES --> EXPLICIT
    EXPLICIT --> COMMANDS
```

### Build Process Flow

```mermaid
flowchart TD
    START[mk invoked] --> PARSE[Parse mkfile]
    PARSE --> TARGET[Identify target]
    TARGET --> DEPS[Check dependencies]
    DEPS --> OUTDATED{Target outdated?}
    OUTDATED -->|Yes| BUILD[Execute build rule]
    OUTDATED -->|No| DONE[Target up to date]
    BUILD --> RECURSIVE[Build dependencies]
    RECURSIVE --> EXECUTE[Execute commands]
    EXECUTE --> DONE
    DONE --> END[Build complete]
```

### Example Mkfile

```makefile
<mkconfig

TARG=hello
OFILES=hello.$O

</sys/src/cmd/mkone

hello.$O: hello.c
        $CC $CFLAGS hello.c
```

## C Compiler Suite

### Compiler Architecture

```mermaid
graph LR
    subgraph "Compilation Pipeline"
        SRC[Source Code]
        CPP[Preprocessor] 
        CC[Compiler]
        OPT[Optimizer]
        ASM[Assembler]
        OBJ[Object File]
    end
    
    subgraph "Linking"
        OBJS[Object Files]
        LIBS[Libraries]
        LD[Linker]
        EXE[Executable]
    end
    
    SRC --> CPP
    CPP --> CC
    CC --> OPT
    OPT --> ASM
    ASM --> OBJ
    
    OBJ --> OBJS
    LIBS --> LD
    OBJS --> LD
    LD --> EXE
```

### Cross-Compilation Support

```mermaid
graph TB
    subgraph "Host Architectures"
        X86[386 - Intel x86]
        AMD64[amd64 - x86-64]
        ARM[arm - ARM]
        PPC[power - PowerPC]
        SPARC[sparc - SPARC]
        MIPS[mips - MIPS]
    end
    
    subgraph "Compiler Variants"
        C8A[8a - 386 assembler]
        C6A[6a - amd64 assembler] 
        C5A[5a - ARM assembler]
        C9A[9a - PowerPC assembler]
        C7A[7a - SPARC assembler]
        CVA[va - MIPS assembler]
    end
    
    X86 --> C8A
    AMD64 --> C6A
    ARM --> C5A
    PPC --> C9A
    SPARC --> C7A
    MIPS --> CVA
```

### Library Structure

```mermaid
graph TB
    subgraph "Core Libraries"
        LIBC[libc - C Runtime]
        LIB9P[lib9p - 9P Protocol]
        LIBTHREAD[libthread - Threading]
        LIBDRAW[libdraw - Graphics]
    end
    
    subgraph "System Libraries"  
        LIBAUTH[libauth - Authentication]
        LIBSEC[libsec - Security/Crypto]
        LIBIP[libip - Networking]
        LIBDISK[libdisk - Disk I/O]
    end
    
    subgraph "Utility Libraries"
        LIBBIO[libbio - Buffered I/O]
        LIBREGEXP[libregexp - Regular Expressions]
        LIBHTML[libhtml - HTML Processing]
        LIBMP[libmp - Multiple Precision Arithmetic]
    end
    
    LIBC --> LIBTHREAD
    LIBC --> LIBBIO
    LIB9P --> LIBAUTH
    LIBDRAW --> LIBTHREAD
    LIBAUTH --> LIBSEC
```

## Debugging Environment

### Acid Debugger

```mermaid
graph TB
    subgraph "Acid Components"
        CORE[Core Debugger]
        LANG[Acid Language]
        SYMS[Symbol Tables]
        PROC[Process Interface]
    end
    
    subgraph "Debug Features"
        BREAK[Breakpoints]
        TRACE[Tracing]
        INSPECT[Memory Inspection]
        STACK[Stack Traces]
    end
    
    CORE --> LANG
    LANG --> SYMS
    SYMS --> PROC
    
    LANG --> BREAK
    LANG --> TRACE
    LANG --> INSPECT
    LANG --> STACK
```

### Debugging Workflow

```mermaid
sequenceDiagram
    participant Developer
    participant Acid
    participant Process
    participant Kernel
    
    Developer->>Acid: Load program
    Acid->>Process: Attach to process
    Developer->>Acid: Set breakpoint
    Acid->>Process: Insert breakpoint
    Process->>Kernel: Hit breakpoint
    Kernel->>Acid: Debug trap
    Acid->>Developer: Show state
    Developer->>Acid: Inspect variables
    Acid->>Process: Read memory
    Process->>Acid: Return data
    Acid->>Developer: Display values
```

## Programming Environment

### Source Code Organization

```mermaid
graph TB
    subgraph "Source Tree Structure"
        SYS[/sys/src/]
        CMD[cmd/ - Commands]
        LIB[lib*/ - Libraries]
        GAMES[games/ - Games]
        BOOT[boot/ - Boot code]
        NINE[9/ - Kernel]
    end
    
    subgraph "User Source"
        USR[/usr/$user/src/]
        PROJ[Project directories]
        LOCAL[Local modifications]
    end
    
    SYS --> CMD
    SYS --> LIB
    SYS --> GAMES
    SYS --> BOOT
    SYS --> NINE
    
    USR --> PROJ
    USR --> LOCAL
```

### Development Workflow

```mermaid
flowchart TD
    IDEA[Project Idea] --> PLAN[Plan Implementation]
    PLAN --> SETUP[Setup Project Directory]
    SETUP --> CODE[Write Code]
    CODE --> BUILD[Build with mk]
    BUILD --> TEST[Test Program]
    TEST --> DEBUG{Bugs Found?}
    DEBUG -->|Yes| ACID[Debug with acid]
    DEBUG -->|No| DONE[Release]
    ACID --> FIX[Fix Issues]
    FIX --> BUILD
    DONE --> INSTALL[Install System-wide]
```

## Text Processing Tools

### Sam Editor

```mermaid
graph TB
    subgraph "Sam Architecture"
        CMD[Command Interface]
        REGEXP[Regular Expressions]
        STRUCT[Structural Editing]
        MULTI[Multiple Files]
    end
    
    subgraph "Sam Commands"
        ADDR[Addressing]
        EDIT[Edit Commands]
        IO[I/O Commands]
        LOOP[Looping Constructs]
    end
    
    CMD --> REGEXP
    REGEXP --> STRUCT
    STRUCT --> MULTI
    
    CMD --> ADDR
    ADDR --> EDIT
    EDIT --> IO
    IO --> LOOP
```

### Text Processing Pipeline

```mermaid
graph LR
    INPUT[Input Text] --> SED[sed - Stream Editor]
    SED --> AWK[awk - Pattern Processing]
    AWK --> SORT[sort - Sorting]
    SORT --> UNIQ[uniq - Duplicate Removal]
    UNIQ --> OUTPUT[Output]
    
    INPUT --> GREP[grep - Pattern Matching]
    GREP --> CUT[cut - Field Extraction]
    CUT --> OUTPUT
```

## Version Control Integration

### Source Management

```mermaid
graph TB
    subgraph "Plan 9 Source Control"
        REPLICA[replica - Synchronization]
        BUNDLE[bundle - Change Sets]
        HISTORY[History Tracking]
    end
    
    subgraph "External VCS"
        GIT[Git Integration]
        SVN[Subversion]
        CVS[CVS Legacy]
    end
    
    REPLICA --> BUNDLE
    BUNDLE --> HISTORY
    
    GIT --> REPLICA
    SVN --> REPLICA
    CVS --> REPLICA
```

## Plumber System

### Message Routing

```mermaid
graph TB
    subgraph "Plumber Components"
        RULES[Plumbing Rules]
        PORTS[Message Ports]
        ROUTER[Message Router]
        APPS[Applications]
    end
    
    subgraph "Message Flow"
        SRC[Source App] --> MSG[Plumb Message]
        MSG --> RULES
        RULES --> ROUTER
        ROUTER --> PORTS
        PORTS --> DEST[Destination App]
    end
    
    APPS --> SRC
    APPS --> DEST
```

### Plumbing Rules

```mermaid
flowchart TD
    MSG[Plumb Message] --> PATTERN[Pattern Match]
    PATTERN --> ATTR[Extract Attributes]
    ATTR --> ACTION[Determine Action]
    ACTION --> PORT[Select Port]
    PORT --> SEND[Send to Application]
    
    PATTERN -->|No Match| NEXT[Try Next Rule]
    NEXT --> PATTERN
```

## Performance Tools

### Profiling Infrastructure

```mermaid
graph TB
    subgraph "Profiling Tools"
        PROF[prof - Function Profiling]
        COVER[coverage - Code Coverage]
        TRACE[trace - System Tracing]
    end
    
    subgraph "Analysis"
        STATS[Statistical Analysis]
        GRAPHS[Performance Graphs] 
        REPORTS[Profiling Reports]
    end
    
    PROF --> STATS
    COVER --> GRAPHS
    TRACE --> REPORTS
```

## Integration with System

### Development Services

```mermaid
graph TB
    subgraph "System Integration"
        FS[File System Access]
        PROC[Process Management]
        NET[Network Services]
        AUTH[Authentication]
    end
    
    subgraph "Development Tools"
        EDITOR[Editor Integration]
        BUILD[Build System]
        DEBUG[Debugging]
        VERSION[Version Control]
    end
    
    FS --> EDITOR
    PROC --> DEBUG
    NET --> VERSION
    AUTH --> BUILD
```

### Cross-Platform Development

```mermaid
graph TB
    subgraph "Target Platforms"
        PLAN9[Plan 9 Native]
        UNIX[Unix/Linux]
        WINDOWS[Windows]
        EMBEDDED[Embedded Systems]
    end
    
    subgraph "Portability Tools"
        APE[APE - ANSI/POSIX Environment]
        CROSS[Cross Compilers]
        EMULATION[Emulation Layer]
    end
    
    PLAN9 --> CROSS
    UNIX --> APE
    WINDOWS --> EMULATION
    EMBEDDED --> CROSS
```

## Configuration and Customization

### Environment Setup

```bash
# Development environment variables
PLAN9=/
PATH=$PLAN9/bin:$PATH
objtype=386
cputype=386

# Editor preferences  
font=/lib/font/bit/lucidasans/typesetter.6.font
acmeshell=rc
```

### Custom Build Rules

```makefile
# Custom mkfile template
<mkconfig

TARG=myprogram
OFILES=\
    main.$O\
    module1.$O\
    module2.$O\

HFILES=\
    myprogram.h\
    internal.h\

</sys/src/cmd/mkone

myprogram.h: generate.c
    $CC -o generate generate.c
    ./generate > myprogram.h
```

## Best Practices

### Code Organization

```mermaid
graph TB
    subgraph "Project Structure"
        ROOT[Project Root]
        SRC[src/ - Source files]
        INCLUDE[include/ - Headers]
        DOC[doc/ - Documentation] 
        TEST[test/ - Test cases]
        MKFILE[mkfile - Build script]
    end
    
    ROOT --> SRC
    ROOT --> INCLUDE
    ROOT --> DOC
    ROOT --> TEST
    ROOT --> MKFILE
```

### Development Cycle

1. **Planning**: Design interfaces and data structures
2. **Implementation**: Write code in small, testable modules  
3. **Testing**: Use built-in test framework
4. **Documentation**: Document interfaces and usage
5. **Integration**: Ensure compatibility with Plan 9 conventions
6. **Performance**: Profile and optimize critical paths

## References

- `/sys/doc/acme/` - Acme documentation
- `/sys/doc/sam/` - Sam editor documentation  
- `/sys/doc/mk.ms` - Mk build system
- `/sys/doc/compiler.ms` - Plan 9 C compiler
- `/sys/doc/acidpaper.ms` - Acid debugger
- `/sys/man/1/` - User command manual pages
- `/sys/src/` - System source code examples