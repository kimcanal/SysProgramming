/* $begin shellmain */
#include "myshell.h"
#include <errno.h>
#define MAXARGS   128
#define MAXPIPES 10

#define MAXJOBS 64

#include <errno.h>
#include <termios.h>

#define MAXARGS 128
#define MAXPIPES 10

#define MAXJOBS 64

/* Function prototypes */
typedef enum { FG, BG, ST } job_state;

typedef struct {
    int jid; //Job ID 
    pid_t pgid; //process group ID
    char cmdline[MAXLINE]; // cmdilne
    job_state state; // FG(foreground), BG(background), ST(stopped)
} job_t;

job_t jobs[MAXJOBS];
int nextjid = 1;//jid of next job
volatile pid_t fg_pgid = 0;
int pipe_cnt;//number of pipes

void eval(char *cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv);
void execute_pipeline(char *commands[MAXPIPES][MAXARGS], int num_commands);
void sigint_handler(int sig);
void sigtstp_handler(int sig);
void sigchld_handler(int sig);

/* Add job to JOBS */
int add_job(pid_t pgid, job_state state, const char *cmdline) {
    for (int i = 0; i < MAXJOBS; ++i) {
        if (jobs[i].jid == 0) {
            jobs[i].jid = nextjid++;
            jobs[i].pgid = pgid;
            jobs[i].state = state;
            strncpy(jobs[i].cmdline, cmdline, MAXLINE);
            return jobs[i].jid;
        }
    }
    return 0;
}
/* Delete job from JOBS */
void delete_job(pid_t pgid) {
    for (int i = 0; i < MAXJOBS; ++i) {
        if (jobs[i].pgid == pgid) {
            jobs[i].jid = 0;
            jobs[i].pgid = 0;
            jobs[i].state = 0;
            jobs[i].cmdline[0] = '\0';
            break;
        }
    }
}
/* Get job adress by job job ID*/
job_t* get_job_by_jid(int jid) {
    for (int i = 0; i < MAXJOBS; ++i)
        if (jobs[i].jid == jid) return &jobs[i];
    return NULL;
}
/* Get job adress by job PGID*/
job_t* get_job_by_pgid(pid_t pgid) {
    for (int i = 0; i < MAXJOBS; ++i)
        if (jobs[i].pgid == pgid) return &jobs[i];
    return NULL;
}
/* Prints list of jobs */
void list_jobs() {
    for (int i = 0; i < MAXJOBS; ++i) {
        if (jobs[i].jid) {
            printf("[%d] %s %s", jobs[i].jid,
                jobs[i].state == BG ? "RUNNING" :
                jobs[i].state == ST ? "STOPPED" : "FOREGROUND",
                jobs[i].cmdline);
        }
    }
}
/* main */
int main() {
    
    struct sigaction sa_int, sa_tstp, sa_chld;

    // SIGINT (Ctrl-C)
    sa_int.sa_handler = sigint_handler;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa_int, NULL);

    // SIGTSTP (Ctrl-Z)
    sa_tstp.sa_handler = sigtstp_handler;
    sigemptyset(&sa_tstp.sa_mask);
    sa_tstp.sa_flags = SA_RESTART;
    sigaction(SIGTSTP, &sa_tstp, NULL);

    // SIGCHLD
    sa_chld.sa_handler = sigchld_handler;
    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa_chld, NULL);

    char cmdline[MAXLINE]; /* Command line */

    do{
        /* Read */
        printf("CSE4100-SP-P2> ");                   
        if (fgets(cmdline, MAXLINE, stdin) == NULL) {
            if (feof(stdin)) //ERROR
                exit(0); 
        }

        /* Evaluate */
        eval(cmdline);
    } while(1);
}
/* $end shellmain */

/* $begin eval */
/* eval - Evaluate a command line */
void eval(char *cmdline) {
  
    char *argv[MAXARGS]; /* Argument list execvp() */
    char buf[MAXLINE];   /* Holds modified command line */
    int bg;              /* Should the job run in bg or fg? */
    pid_t pid;           /* Process id */

    char *commands[MAXPIPES][MAXARGS] = {NULL}; /* store command of  pipeline*/
    pipe_cnt = 0;/* Number of pipe */ 

    strcpy(buf, cmdline);
    bg = parseline(buf, argv);
    if (bg < 0) return; /* Quote Error!*/
    if (argv[0] == NULL) return; /* Ignore empty lines */

    /* when PIPELINE exist */
    if (pipe_cnt > 0) {
        int cmd_idx = 0, arg_idx = 0;
        for (int i = 0; argv[i] != NULL; i++) {
            if (!strcmp(argv[i], "|")) {
                commands[cmd_idx++][arg_idx] = NULL;
                arg_idx = 0;
            } 
            else {
                commands[cmd_idx][arg_idx++] = argv[i];/* Save command*/
            }
        }
        commands[cmd_idx][arg_idx] = NULL; /* Last command */
        execute_pipeline(commands, pipe_cnt + 1);  /* Execute Pipeline! */
        return;
    }
    // NO pipeline
    if (builtin_command(argv)) return;
    
    pid = fork();
    if (pid == 0) { // Child
        setpgid(0, 0);
        if (execvp(argv[0], argv) < 0) {
            fprintf(stderr, "%s: Command not found.\n", argv[0]);
            exit(0);
        }
    } 
    else if (pid > 0) { // Parent
        setpgid(pid, pid);
        if (!bg) {
            fg_pgid = pid;
            int status;
            if (waitpid(pid, &status, WUNTRACED) > 0) {
                if (WIFSTOPPED(status)) {
                    add_job(pid, ST, cmdline);
                    printf("\n[%d] Stopped %s", nextjid-1, cmdline);
                }
            }
            fg_pgid = 0;
        } 
        else {
            add_job(pid, BG, cmdline);
            printf("[%d] %d %s", nextjid-1, pid, cmdline);
        }
    } 
    else {// pid < 0
        perror("fork error");
    }
}

int builtin_command(char **argv) 
{
    if (!strcmp(argv[0], "exit") || !strcmp(argv[0], "quit")) { /* quit OR exit command */
	    exit(0);  
    }
    if (!strcmp(argv[0], "&")) { /* Ignore singleton & */
	    return 1;    
    }

    if (!strcmp(argv[0], "cd")) { /* cd command */
        //Moving to HOME directory
        if (argv[1] == NULL) {        
            const char* home = getenv("HOME");
            if(chdir(home) != 0){//ERROR
                fprintf(stderr, "cd: Failed to move to HOME directory\n");
            }
        } 
        else if (chdir(argv[1]) != 0) {//ERROR
            fprintf(stderr, "cd: %s: No such file or directory\n", argv[1]);
        }
        return 1; 
        //DONE_BUILT_IN_COMMAND
    }
    //PHASE 3 
    if (!strcmp(argv[0], "jobs")) {
            list_jobs();
            return 1;
    }
    if (!strcmp(argv[0], "bg")) {
        if (argv[1] == NULL) { printf("Usage: bg <job>\n"); return 1; }
        int jid = atoi(argv[1]);
        job_t *job = get_job_by_jid(jid);
        if (job && job->state == ST) {
            kill(-job->pgid, SIGCONT);
            job->state = BG;
            printf("[%d] %s", job->jid, job->cmdline);
        }
        return 1;
    }
    if (!strcmp(argv[0], "fg")) {
        if (argv[1] == NULL) { printf("Usage : fg <job>\n"); return 1; }
        if(argv[1][0] == '%') argv[1][0] = ' ';
        else{
            printf("%s: (%s) - Operation not permitted\n", argv[0], argv[1]);
            return 1;
        }
        int jid = atoi(argv[1]);
        job_t *job = get_job_by_jid(jid);
        if (job) {
            kill(-job->pgid, SIGCONT);
            job->state = FG;
            fg_pgid = job->pgid;
            int status;
            waitpid(-job->pgid, &status, WUNTRACED);
            fg_pgid = 0;
            if (WIFSTOPPED(status)) {
                job->state = ST;
            } 
            else {
                delete_job(job->pgid);
            }
        }
        return 1;
    }
    if (!strcmp(argv[0], "kill")) {
        if(argv[1][0] == '%') argv[1][0] = ' ';
        else{
            printf("%s: (%s) - Operation not permitted\n", argv[0], argv[1]);
            return 1;
        }
        if (argv[1] == NULL) { 
            printf("usage : kill <job>\n"); 
            return 1; 
        }
        int jid = atoi(argv[1]);
        job_t *job = get_job_by_jid(jid);
        if (job) {
            kill(-job->pgid, SIGKILL);
            delete_job(job->pgid);
        }
        return 1;
    }

    return 0;    
}
int parseline(char *buf, char **argv) {
    int argc = 0;
    int bg = 0;
    char *current = buf;
    char *start = buf;
    char in_quote = 0;
    char escape = 0;
    pipe_cnt = 0;
    buf[strlen(buf)-1] = ' ';
    while (*current) {
        if (escape) {
            *start++ = *current++;
            escape = 0;
            continue;
        }
        if (*current == '\\') {
            escape = 1;
            current++;
            continue;
        }
        if (in_quote) {
            if ((in_quote == 1 && *current == '\'') || (in_quote == 2 && *current == '"')) {
                in_quote = 0;
                current++;
            } else {
                *start++ = *current++;
            }
        } else {
            switch(*current) {
                case '\'': in_quote = 1; current++; break;
                case '"':  in_quote = 2; current++; break;
                case '|':
                    *start = '\0';
                    if (start > buf) argv[argc++] = buf;
                    argv[argc++] = "|";
                    pipe_cnt++;
                    current++;
                    buf = current;
                    start = buf;
                    break;
                case ' ':
                    *start = '\0';
                    if (start > buf) argv[argc++] = buf;
                    current++;
                    while (*current == ' ') current++;
                    buf = current;
                    start = buf;
                    break;
                default:
                    *start++ = *current++;
            }
        }
    }
    if (in_quote) {
        printf("error: unmatched quotes\n");
        return -1;
    }
    *start = '\0';
    if (start > buf) argv[argc++] = buf;
    argv[argc] = NULL;
    if (argc == 0) return 1;
    if ((bg = (*argv[argc-1] == '&')) != 0)
        argv[--argc] = NULL;
    return bg;
}

void execute_pipeline(char *commands[MAXPIPES][MAXARGS], int num_commands) {
    int prev_pipe[2];
    int next_pipe[2];
    pid_t pid;
    for (int i = 0; i < num_commands; i++) {
        if (i < num_commands - 1) pipe(next_pipe);
        pid = fork();
        if (pid == 0) { // Child
            setpgid(0, 0);
            if (i > 0) {
                dup2(prev_pipe[0], STDIN_FILENO);
                close(prev_pipe[0]);
                close(prev_pipe[1]);
            }
            if (i < num_commands - 1) {
                close(next_pipe[0]);
                dup2(next_pipe[1], STDOUT_FILENO);
                close(next_pipe[1]);
            }
            if(execvp(commands[i][0], commands[i]) < 0){
                fprintf(stderr, "%s: Command not found\n", commands[i][0]);
                exit(0);
            }
        }
        if (i > 0) {
            close(prev_pipe[0]);
            close(prev_pipe[1]);
        }
        if (i < num_commands - 1) {
            prev_pipe[0] = next_pipe[0];
            prev_pipe[1] = next_pipe[1];
        }
    }
    for (int i = 0; i < num_commands; i++)
        wait(0);
}

void sigint_handler(int sig) {
    if (fg_pgid != 0) {
        kill(-fg_pgid, SIGINT);
    }
}

void sigtstp_handler(int sig) {
    if (fg_pgid != 0) {
        kill(-fg_pgid, SIGTSTP);
    }
}

void sigchld_handler(int sig) {
    int olderrno = errno;
    pid_t pid;
    int status;
    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) > 0) {
        job_t *job = get_job_by_pgid(pid);
        if (!job) continue;
        if (WIFEXITED(status) || WIFSIGNALED(status)) {
            delete_job(pid);
        } else if (WIFSTOPPED(status)) {
            job->state = ST;
        } else if (WIFCONTINUED(status)) {
            job->state = BG;
        }
    }
    errno = olderrno;
}
