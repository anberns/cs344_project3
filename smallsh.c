/*****************************************************************************
* smallsh.c 
* Aaron Berns
*
*****************************************************************************/ 

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h>

#define NUMROOMS 7

  
int main()
{
    // catch SIGINT and ignore for shell
    struct sigaction ignore_action = {0};
    ignore_action.sa_handler = SIG_IGN;
    sigaction(SIGINT, &ignore_action, NULL);

    char *userArgs[512];
    char cwd[2048];

    char *input = NULL;
    int charsEntered, i, exitStatus, termSignal;
    size_t buffersize = 0;

    // spawn test process
    pid_t spawnpid = -5;
    int childExitMethod = -5;
    
    spawnpid = fork();
    switch (spawnpid)
    {
        case -1:
            exit(1);
            break;
        case 0:
            //for (i = 0; i < 10; ++i) {
                printf("Yo Yo!\n");
                sleep(1);
            //}
            exit(8);
            break;
        default:
            waitpid(spawnpid, &childExitMethod, 0);
            break;
    }
    fflush(stdin);

    // run shell
    while (1) {

        // get user command
        printf(":");
        charsEntered = getline(&input, &buffersize, stdin);

        // remove line feed
        input[charsEntered-1] = '\0';

        // parse for command and possible args
        i = 1;
        userArgs[0] = strtok(input, " ");
        while (userArgs[i-1] != NULL) {
            userArgs[i] = strtok(NULL, " ");
            i++;
        }
        
        // check for and run built in commands
        // exit
        if (strcmp(userArgs[0], "exit") == 0) {

            // free memory
            free(input);

            // kill all processes, help from ibm.com
            kill(0, 15);;

            break;
        }

        // status
        if (strcmp(userArgs[0], "status") == 0) {
            
            //check exit status
            if (WIFEXITED(childExitMethod) != 0) {
                exitStatus = WEXITSTATUS(childExitMethod);    
                printf("Exit status %d\n", exitStatus);
            }
            else if (WIFSIGNALED(childExitMethod) != 0) {
                termSignal = WTERMSIG(childExitMethod);
                printf("Terminating signal %d\n", termSignal);
            }
        }

        // cd
        if (strcmp(userArgs[0], "cd") == 0) {
            if (userArgs[1] == NULL) {
                chdir(getenv("HOME"));
            }
            else {
                chdir(userArgs[1]);
            }
            getcwd(cwd, sizeof(cwd));
            printf("%s\n", cwd);
        }
    }
    

	return 0;
}

