#Custom Linux Shell - System Programming Project 2
Course: CSE4100 - System Programming
Student ID: 20211518  

---
## Overview

This project implements a custom Linux shell through **three development phases**, 
each adding new system-level functionality including command execution, pipelining, and job control.

The shell provides a prompt (`CSE4100-SP-P2>`) and waits for user input, 
then parses and executes the command accordingly.

---

## ✅ Phase 1 - Basic Shell with `fork()` & `execvp()`

### Features:
- Executes standard commands such as `cd`, `ls`, `mkdir`, `touch`, `cat`, `echo`, and `exit`.
- Uses `fork()` to create a child process and `execvp()` to run the given command.
- The parent process waits for the child to finish using `waitpid()`.
- Built-in commands like `cd` and `exit` are handled without creating child processes.
- Handles quoted strings and escape characters (`'`, `"`, `\`) similarly to a real shell.

---

To compile and run the shell:

make
./myshell
