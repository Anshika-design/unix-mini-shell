# Unix Mini Shell

A lightweight Unix-like shell built from scratch in **C** to understand Linux process management and system programming.

## Features

- Execute Linux commands using `fork()` and `execvp()`
- Built-in commands:
  - `cd`
  - `exit`
  - `jobs`
  - `fg`
  - `bg`
- Background process execution using `&`
- Process and job tracking
- Signal handling using `SIGCHLD`, `SIGINT`, `SIGTSTP`, and `SIGCONT`
- Single and multiple pipes using `|`
- Input redirection using `<`
- Output redirection using `>`
- Append output using `>>`

## Concepts Used

This project provides hands-on implementation of:

- Process creation and management
- Signals and child-process handling
- Pipes and inter-process communication
- File descriptors
- Input/output redirection
- Background processes and job management
- Unix system calls

### Key System Calls

```text
fork()      → Create a child process
execvp()    → Execute a program
waitpid()   → Wait for child processes
pipe()      → Create communication channels
dup2()      → Redirect file descriptors
open()      → Open files for I/O
kill()      → Send signals to processes
setpgid()   → Create process groups
```

## How It Works

The shell follows a basic command-processing flow:

```text
User Input
    ↓
Command Parsing
    ↓
Check Built-in Commands
    ↓
Detect Pipes / Redirection / Background Jobs
    ↓
Create Process
    ↓
fork() → execvp()
    ↓
Manage Process
    ↓
waitpid() / Job Tracking / Signals
```

## Example

### Run a command

```bash
Mini Shell> ls
```

### Use a pipe

```bash
Mini Shell> ls | grep .c
```

### Run a process in the background

```bash
Mini Shell> sleep 20 &
```

### View running jobs

```bash
Mini Shell> jobs
```

### Bring a job to foreground

```bash
Mini Shell> fg 1
```

### Redirect output

```bash
Mini Shell> ls > output.txt
```

### Append output

```bash
Mini Shell> ls >> output.txt
```

### Redirect input

```bash
Mini Shell> sort < input.txt
```

## Real-World Application

The mechanisms implemented in this project are relevant to command-line environments used in **Linux servers, Raspberry Pi systems, IoT gateways, and embedded Linux devices**.

A lightweight shell or task runner can be used to launch and manage processes, run background services, and connect utilities through pipelines.

For example:

```bash
./sensor_logger &
cat sensor_data.txt | grep ERROR | wc -l
```

This project can serve as a foundation for building a specialized **Linux task runner or command interface for embedded and resource-constrained systems**.

## Getting Started

### Compile

```bash
gcc shell.c -o shell
```

### Run

```bash
./shell
```

The shell will start with:

```text
Mini Shell>
```

## Limitations

This is an **educational mini-shell**, not a complete replacement for Bash.

Current limitations include:

- Basic whitespace-based command parsing
- Limited argument handling
- Fixed-size job table
- No command history
- No environment-variable expansion
- No wildcard/globbing support
- No advanced shell scripting features

## Future Improvements

- Improved command parsing
- Command history
- Environment variable support
- Wildcard expansion
- Better terminal job control
- Automated testing
- Modular source files
- Support for `&&` and `||`

## Tech Stack

**Language:** C

**Platform:** Linux / Unix-like systems

**APIs:** POSIX / Unix system calls

## Author

**Anshika Dhiman**

Built as a hands-on **Operating Systems and Unix system programming project**.
