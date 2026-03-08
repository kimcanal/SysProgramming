/* $begin shellmain */
#include "myshell.h"
#include <errno.h>
#define MAXARGS   128

/* Function prototypes */
void eval(char *cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv);

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
    
    strcpy(buf, cmdline);
    bg = parseline(buf, argv);
    
    if(bg < 0) return;/* Quote Error!*/

    if (argv[0] == NULL) return; /* Ignore empty lines */

    // NOT_BUILT_IN_COMMAND
    if (!builtin_command(argv)) { //quit -> exit(0), & -> ignore, other -> run
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
    char *delim;         /* Points to first space delimiter */
    int argc;            /* Number of args */
    int bg;              /* Background job? */

    /* flag of Quotes: Check whether quote is closed*/
    char doubleQuote = 0;
    char singleQuote = 0;

    for(int i = 0; i < strlen(buf); i++){
        if(doubleQuote){
            //doubleQuote
            if(buf[i] == '\"'){
                buf[i] = ' ';
                doubleQuote = !doubleQuote;
            }
        }
        else if (singleQuote){
                //singleQuote
                if(buf[i] == '\''){
                    buf[i] = ' ';
                    singleQuote = !singleQuote;
                }
       }
       else{
            //doubleQuote
            if(buf[i] == '\"'){
                buf[i] = ' ';
                doubleQuote = !doubleQuote;
            }
            //singleQuote
            else if(buf[i] == '\''){
                buf[i] = ' ';
                singleQuote = !singleQuote;
            }
            //escape character
            else if (buf[i] == '\\'){
                buf[i++] = ' ';
            }
        }
    }
       
    
    if(doubleQuote | singleQuote){
        printf("error: unmatched quotes\n");
        printf("----------------------------------------\n");
        return -1;
    }
    

    buf[strlen(buf)-1] = ' ';  /* Replace trailing '\n' with space */
    while (*buf && (*buf == ' ')){ /* Ignore leading spaces */
        buf++; 
    }

    /* Build the argv list */
    argc = 0;
    while ((delim = strchr(buf, ' '))) {
	    argv[argc++] = buf;
	    *delim = '\0';
	    buf = delim + 1;
	    while (*buf && (*buf == ' ')) buf++; /* Ignore spaces */
    }
    argv[argc] = NULL;
    
    if (argc == 0) return 1; /* Ignore blank line */

    /* Should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0)
	    argv[--argc] = NULL;

    return bg;
}
/* $end parseline */