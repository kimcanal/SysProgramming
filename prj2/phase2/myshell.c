/* $begin shellmain */
#include "myshell.h"
#include <errno.h>
#define MAXARGS   128

#define MAXPIPES 10

/* Function prototypes */
void eval(char *cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv);

void execute_pipeline(char *commands[MAXPIPES][MAXARGS] , int num_commands);
int pipe_cnt;


int main() 
{
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
void eval(char *cmdline) 
{
    char *argv[MAXARGS]; /* Argument list execvp() */
    char buf[MAXLINE];   /* Holds modified command line */
    int bg;              /* Should the job run in bg or fg? */
    pid_t pid;           /* Process id */

    char *commands[MAXPIPES][MAXARGS] = {NULL}; // 정적 배열로 파이프라인 명령어 저장
    pipe_cnt = 0;/* Number of pipe */
    
    strcpy(buf, cmdline);
    bg = parseline(buf, argv);
    
    if(bg < 0) return;/* Quote Error!*/
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
                commands[cmd_idx][arg_idx++] = argv[i]; // 명령어 저장
            }
        }
        commands[cmd_idx][arg_idx] = NULL; /* No more command left */
        execute_pipeline(commands, pipe_cnt + 1); /* Execute commands saved in pipline*/
    }

    else{  
		// NOT_BUILT_IN_COMMAND
        if (!builtin_command(argv)) { 
            pid = fork();
            /* Child Process Start*/
            if (pid == 0) { 
                if (execvp(argv[0], argv) < 0) { // Execution
                    fprintf(stderr, "%s: Command not found.\n", argv[0]);
                    exit(0); 
                    /* Child Process Terminated */
                }
            } 
        /* Parent Process */
         else{ 
            /* Parent waits for foreground job to terminate */
            if (!bg) { 
                int status;
                waitpid(pid, &status, 0);
            } 
            else    //when there is background process!
                    printf("%d %s", pid, cmdline);
            }
        }
    }
}

/* If first arg is a builtin command, run it and return true */
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

    return 0;                     /* Not a builtin command */
}
/* $end eval */

/* $begin parseline */
/* parseline - Parse the command line and build the argv array */
int parseline(char *buf, char **argv) 
{
    int argc = 0;
    int bg = 0;
    char *current = buf;
    char *start = buf;

    char in_quote = 0; // 0: none, 1: single quote, 2: double quote
    char escape = 0; 
    pipe_cnt = 0; // number of pipes

    
    buf[strlen(buf)-1] = ' ';  /* Replace trailing '\n' with space */

    while (*current) {
        if (escape) { /* 이스케이프 문자 처리 */
            *start++ = *current++;
            escape = 0;
            continue;
        }

        if (*current == '\\') { /* 이스케이프 시작 */
            escape = 1;
            current++;
            continue;
        }

        if (in_quote) { /* inside quotes */
            if ((in_quote == 1 && *current == '\'') || (in_quote == 2 && *current == '"')) {
                in_quote = 0; // quote closed
                current++;
            } 
            else {
                *start++ = *current++;
            }
        } 
        else { /* outside of quotes */
            switch(*current) {
                case '\'':  // single quote
                    in_quote = 1;
                    current++;
                    break;
                case '\"':   // double quote
                    in_quote = 2;
                    current++;
                    break;
                case '|':   // pipe
                    *start = '\0';
                    if (start > buf) 
                        argv[argc++] = buf;
                    argv[argc++] = "|";
                    pipe_cnt++;
                    current++;
                    buf = current;
                    start = buf;
                    break;
                case ' ':   // 공백 처리
                    *start = '\0';
                    if (start > buf) argv[argc++] = buf;
                    current++;
                    while (*current == ' ') current++;
                    buf = current;
                    start = buf;
                    break;
                default:    // normal character input
                    *start++ = *current++;
            }
        }
    } 
    /* Quote ERROR : unmatched quotes*/
    if (in_quote) {
        printf("error: unmatched quotes\n");
        return -1;
    }

    *start = '\0';
    if (start > buf) 
        argv[argc++] = buf;
    argv[argc] = NULL;

    if (argc == 0) return 1; /* Ignore blank line */
    
    /* Should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0)
    argv[--argc] = NULL;


    return bg;
}

/* $end parseline */


void execute_pipeline(char *commands[MAXPIPES][MAXARGS], int num_commands) 
{
    int prev_pipe[2];
    int next_pipe[2];
    pid_t pid;

    for (int i = 0; i < num_commands; i++) {
        if (i < num_commands - 1) pipe(next_pipe);
        pid = fork();
        if (pid == 0) {/* Child Process */
            /* 입력 리다이렉션 */
            if (i > 0) {
                dup2(prev_pipe[0], STDIN_FILENO);
                close(prev_pipe[0]);
                close(prev_pipe[1]);
            }

            /* 출력 리다이렉션 */
            if (i < num_commands - 1) {
                close(next_pipe[0]);
                dup2(next_pipe[1], STDOUT_FILENO);
                close(next_pipe[1]);
            }

            if(execvp(commands[i][0], commands[i]) < 0){
                fprintf(stderr, "%s: Command not found\n", commands[i][0]);
                exit(0);
            }
            /* Child Process Terminate*/
        }

        /* Parent Process */
        if (i > 0) {
            close(prev_pipe[0]);
            close(prev_pipe[1]);
        }
        if (i < num_commands - 1) {
            prev_pipe[0] = next_pipe[0];
            prev_pipe[1] = next_pipe[1];
        }
    }

    /* Parent Process : Wait until every child process is terminated */
    for (int i = 0; i < num_commands; i++) 
        wait(0);
}