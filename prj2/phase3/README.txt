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

## ✅ Phase 3 – Background Processes & Job Control

### Features:

- **Background Execution with `&`**
  - When a command ends with `&`, it is executed in the background.
  - The shell does not wait for background jobs to complete and immediately shows the prompt.
  - Example:
    ```bash
    sleep 10 &
    ```

- **Process Group & Job Management**
  - Each background process is assigned a unique process group ID using `setpgid()`.
  - Background jobs are tracked using a `job_t` struct containing:
    - `jid` (Job ID)
    - `pgid` (Process Group ID)
    - `cmdline` (Original command line)
    - `state` (FG / BG / ST)

- **Job List**
  - A maximum of 64 jobs (`MAXJOBS`) are stored in an array.
  - Each job can be in one of three states:
    - `FG`: Foreground
    - `BG`: Background
    - `ST`: Stopped

- **Signal Handling**
  - `SIGINT` (`Ctrl+C`) is forwarded to the foreground process group.
  - `SIGTSTP` (`Ctrl+Z`) stops the foreground job and marks it as stopped.
  - `SIGCHLD` is used to reap terminated or stopped child processes and update job status.

- **Built-in Job Control Commands**
  - `jobs`: Lists current background and stopped jobs.
  - `fg %<jid>`: Brings a stopped background job to the foreground.
  - `bg %<jid>`: Resumes a stopped job in the background.
  - `kill %<jid>`: Sends `SIGKILL` to terminate a job.

### Example Usage:

```bash
sleep 10 &
[1] 12345 sleep 10

jobs
[1] RUNNING sleep 10

fg %1
sleep 10
^Z
[1] Stopped sleep 10

bg %1
[1] sleep 10
