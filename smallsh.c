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

void execute(char **argv) {
    if (execvp(*argv, argv) < 0) {
        perror("Could not execute command");
        exit(1);
    }
}
  
int main()
{
    // catch SIGINT and ignore for shell
    struct sigaction ignore_action = {0};
    ignore_action.sa_handler = SIG_IGN;
    sigaction(SIGINT, &ignore_action, NULL);

    char *userArgs[512];
    char cwd[2048];

    char *input = NULL;
    int charsEntered, i, childExitMethod, exitStatus, termSignal, redirResult,
    sourceFD, targetFD;

    // redirection command locations in userArgs[]
    int outIndex, inIndex, backIndex, lastIndex, curIndex; 

    size_t buffersize = 0;

    

    // run shell
    while (1) {

        // get user command
        printf(":");
        fflush(stdout);
        charsEntered = getline(&input, &buffersize, stdin);

        // remove line feed
        input[charsEntered-1] = '\0';

        // ignore comments and blank lines
        if (input[0] != '#' && strcmp(input, "") != 0) {

            // parse for command and possible args
            outIndex = inIndex = backIndex = 0;
            i = 1;
            userArgs[0] = strtok(input, " ");
            while (userArgs[i-1] != NULL) {
                userArgs[i] = strtok(NULL, " ");

                // look for <, >, & and store index
                if (userArgs[i] != NULL) {
                    if (strcmp(userArgs[i], "<") == 0) {
                        inIndex = i;
                      //  printf("inIndex = %d\n", inIndex);
                    } else if (strcmp(userArgs[i], ">") == 0) {
                        outIndex = i;
                     //   printf("outIndex = %d\n", outIndex);
                    } else if (strcmp(userArgs[i], "&") == 0) {
                        backIndex = i;
                    }

                    // keep track of last arg
                    lastIndex = i+1;
                    //printf("lastIndex = %d\n", lastIndex);

                }

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
            else if (strcmp(userArgs[0], "status") == 0) {
                
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
            else if (strcmp(userArgs[0], "cd") == 0) {
                if (userArgs[1] == NULL) {
                    chdir(getenv("HOME"));
                }
                else {
                    chdir(userArgs[1]);
                }
                getcwd(cwd, sizeof(cwd));
                printf("%s\n", cwd);
            }

            // pass other commands
            else {

                // spawn test process
                pid_t spawnpid = -5;
                childExitMethod = -5;
                
                int stopLoop = 10;
                spawnpid = fork();
                switch (spawnpid)
                {
                    case -1:
                        exit(1);
                        break;
                    case 0:
                        // perform redirection if needed
                        // output
                        if (outIndex != 0) {
                            targetFD = open(userArgs[outIndex + 1], O_WRONLY |
                            O_CREAT | O_TRUNC, 0644);
                            if (targetFD == -1) {
                                perror("target open()"); 
                                exit(1); 
                            }
                            redirResult = dup2(targetFD, 1);
                            if (redirResult == -1) {
                                perror("target dup2()");
                                exit(2);
                            }

                            // remove >, target name from userArgs[]
                            userArgs[outIndex] = NULL;
                            userArgs[outIndex + 1] = NULL;
                            if (outIndex + 2 != lastIndex) {
                                curIndex = outIndex;
                                while (curIndex + 2 != lastIndex && stopLoop != 0) {
                                    userArgs[curIndex] = userArgs[curIndex + 2];
                                    userArgs[curIndex + 2] = NULL;
                                    curIndex++;
                                    stopLoop--;
                                }
                                lastIndex -= 2;
                            }
                        }

                        // input 
                        if (inIndex != 0) {
                            sourceFD= open(userArgs[inIndex + 1], O_RDONLY);
                            if (sourceFD == -1) {
                                perror("source open()"); 
                                exit(1); 
                            }
                            redirResult = dup2(sourceFD, 0);
                            if (redirResult == -1) {
                                perror("source dup2()");
                                exit(2);
                            }

                            // remove <, source name from userArgs[]
                            userArgs[inIndex] = NULL;
                            userArgs[inIndex + 1] = NULL;
                            stopLoop = 10;
                            if (inIndex + 2 != lastIndex && stopLoop != 0) {
                                curIndex = inIndex;
                                while (curIndex + 2 != lastIndex) {
                                    userArgs[curIndex] = userArgs[curIndex + 2];
                                    userArgs[curIndex + 2] = NULL;
                                    curIndex++;
                                    stopLoop--;
                                }
                                lastIndex -= 2;
                            }
                        }
                        
                        execute(userArgs);
                        break;
                    default:
                        waitpid(spawnpid, &childExitMethod, 0);
                        break;
                }
            }
        }
    }

	return 0;
}

