# Custom Linux Shell - System Programming Project 2
Course: CSE4100 - System Programming
Student ID: 20211518

---
## Overview

This project implements a custom Linux shell through **three development phases**,
each adding new system-level functionality including command execution, pipelining, and job control.

The shell provides a prompt (`CSE4100-SP-P2>`) and waits for user input,
then parses and executes the command accordingly.

---

## ✅ Phase 2 - Command Pipelining (`|`)

### Features:
- Implements command pipelines using the pipe symbol (`|`).
- Parses the command line into multiple segments and executes them with pipes.
- For example:
  ```bash
  ls | grep .c | wc -l
  ```
