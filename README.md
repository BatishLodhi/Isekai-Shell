# Isekai-Shell
A Customized Shell made with the help of C language

# Isekai Shell - Gateway to a Parallel Digital Universe

Welcome to **Isekai Shell**! This custom shell is designed to enhance your command-line experience with an intuitive interface for executing commands, managing processes, job control, I/O redirection, and much more. Here, you’ll find everything you need to install, use, and understand the Isekai Shell’s features.

## Overview

Isekai Shell is a lightweight shell that simplifies various command-line tasks while providing advanced functionalities like:
- Basic command execution
- Process management
- Job control (background and foreground processes)
- I/O redirection
- Command piping

## Table of Contents

- [Design and Architecture](#design-and-architecture)
- [Key Algorithms and Methodologies](#key-algorithms-and-methodologies)
- [Installation Instructions](#installation-instructions)
- [Usage Instructions](#usage-instructions)
- [Functionalities](#functionalities)
- [Conclusion](#conclusion)

## Design and Architecture

Isekai Shell is designed with a modular architecture that makes command processing efficient and user-friendly. Key components include:

- **Command Execution:** Utilizes `execvp` to execute external commands, while built-in commands like `cd`, `exit`, `history`, and job control commands (`fg`, `bg`) are handled separately for efficient management.
- **Process Management:** Maintains a job table to keep track of background and foreground processes, each represented by a `Job` struct with the process ID (PID), command string, and status (running or stopped).
- **Signal Handling:** Captures `SIGINT` to prevent abrupt termination and provide a user-friendly exit.
- **I/O Redirection:** Supports input (`<`) and output (`>`) redirection, allowing seamless file management.
- **Piping:** Enables command chaining via the `|` operator, passing output from one command to the input of the next.
- **History Management:** Stores command history for easy access to previous commands.

## Key Algorithms and Methodologies

- **Command Parsing:** Tokenizes input using `strtok`, allowing processing of commands, pipes, and redirection.
- **Forking and Executing:** Commands are executed by forking a new process with `fork()` and executing in the child process, with the parent process waiting to maintain synchronization.
- **Signal Handling:** Uses `signal` to register the `SIGINT` handler for better user experience by gracefully handling interruptions.

## Installation Instructions

To install Isekai Shell, follow these steps:

1. **Navigate to the Directory**: Open a terminal and change to the project directory.
   ```bash
   cd /path/to/shell-directory
   ```
2. **Compile the Code**: Use GCC to compile the source code.
   ```bash
   gcc isekai.c -o isekai_shell
   ```
3. **Run the Shell**: Start Isekai Shell by executing the compiled binary.
   ```bash
   ./isekai_shell
   ```

## Usage Instructions

- **Executing Commands**: Type any command at the prompt (`isekai>`) and press Enter.
- **Commands with Arguments**: Follow syntax, e.g., `kill <pid>`, `cd <directory>`.
- **Help Command**: Type `help` to get a list of commands and their usage.

## Functionalities

Isekai Shell supports the following commands and functionalities:

- **Basic Commands**: Execute standard commands (`ls`, `pwd`, `echo`, etc.) directly.
- **Changing Directories**: Use `cd <directory>` to navigate directories. Defaults to the home directory if no directory is specified.
- **Process Management**: List running processes with `ps`, and terminate processes with `kill <pid>`.
- **Job Control**: Manage background jobs with `bg <job_id>` (resume in background) and `fg <job_id>` (bring to foreground).
- **History Management**: Use `history` to see a list of previously executed commands.
- **I/O Redirection**: Redirect output to a file with `>`, and read input from a file with `<`.
- **Piping**: Use `|` to pass output from one command as input to another.

## Conclusion

Isekai Shell is crafted for efficiency and ease of use, offering users advanced command-line capabilities in a lightweight interface. For further assistance, use the `help` command or refer to this documentation for detailed guidance.

Enjoy exploring the command line with Isekai Shell!
