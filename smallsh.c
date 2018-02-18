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
    // catch SIGINT and ignore for shell and background processes
    struct sigaction ignore_action = {0};
    ignore_action.sa_handler = SIG_IGN;
    sigaction(SIGINT, &ignore_action, NULL);

    // get shell pid and convert to string;
	int pid = getpid();
	char sPid[6];
	sprintf(sPid, "%d", pid);

    char *userArgs[512];
    char cwd[2048];
    pid_t childPid;

    char *input = NULL;
    int charsEntered, i, childExitMethod, exitStatus, termSignal, redirResult,
    sourceFD, targetFD;

    pid_t openJobs[25];
    for (i = 0; i < 25; ++i) {
        openJobs[i] = -5;
    }
    int numOpen = 0;
    // redirection command locations in userArgs[]
    int outIndex, inIndex, backIndex, lastIndex, curIndex; 

    size_t buffersize = 0;

    

    // run shell
    while (1) {

        // check for waiting children
        if (numOpen > 0) {
            for (i = 0; i < numOpen; ++i) {
                childPid = waitpid(openJobs[i], &childExitMethod, WNOHANG);
                if (childPid != 0) {
                    
                    //check exit status
                    if (WIFEXITED(childExitMethod) != 0) {
                        exitStatus = WEXITSTATUS(childExitMethod);    
                    }
                    printf("Background process %d returned with exit "
                    "status %d\n", childPid, exitStatus);
                    openJobs[i] = openJobs[numOpen -1];
                    openJobs[numOpen -1] = -5;
                    numOpen--;
                }
            }
        }

        // get user command
        fflush(stdout);
        printf(":");
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

                // look for <, >, & and store index or $$ and expand
                if (userArgs[i] != NULL) {
                    if (strcmp(userArgs[i], "<") == 0) {
                        inIndex = i;
                    } else if (strcmp(userArgs[i], ">") == 0) {
                        outIndex = i;
                    } else if (strcmp(userArgs[i], "&") == 0) {
                        backIndex = i;
                    } else if (strcmp(userArgs[i], "$$") == 0) {
                        userArgs[i] = NULL;
                        userArgs[i] = sPid;
                    }

                    // keep track of last arg
                    lastIndex = i + 1;
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

                        // redirect to background to dev/null if unspecified
                        // help from stackoverflow #14846768
                        if (backIndex == lastIndex - 1 && inIndex == 0 && outIndex == 0) {
                            sourceFD = open("/dev/null", O_RDONLY);
                            if (sourceFD == -1) {
                                perror("source /dev/null open()"); 
                                exit(1); 
                            }
                            redirResult = dup2(sourceFD, 0);
                            if (redirResult == -1) {
                                perror("source /dev/null dup2()");
                                exit(2);
                            }
                            targetFD = open("/dev/null", O_WRONLY);
                            if (targetFD == -1) {
                                perror("target /dev/null open()"); 
                                exit(1); 
                            }
                            redirResult = dup2(targetFD, 0);
                            if (redirResult == -1) {
                                perror("target /dev/null dup2()");
                                exit(2);
                            }
                        }
                        // remove & from userArgs[]
                        if (backIndex != 0) {
                            userArgs[backIndex] = NULL;
                            userArgs[backIndex + 1] = NULL;
                            stopLoop = 10;
                            if (backIndex + 1 != lastIndex && stopLoop != 0) {
                                curIndex = backIndex;
                                while (curIndex + 1 != lastIndex) {
                                    userArgs[curIndex] = userArgs[curIndex + 1];
                                    userArgs[curIndex + 1] = NULL;
                                    curIndex++;
                                    stopLoop--;
                                }
                                lastIndex -= 1;
                            }
                        } 
                        // set foreground process SIGINT to default
                        if (backIndex == 0) {
                            struct sigaction SIGINT_action = {0};
                            SIGINT_action.sa_handler = SIG_DFL;
                            sigaction(SIGINT, &SIGINT_action, NULL);
                        }    
                        execute(userArgs);
                        
                        break;
                    default:

                        // check for &, run in background
                        if (backIndex < lastIndex -1) {
                            childPid = waitpid(spawnpid, &childExitMethod, 0);
                            if (WIFSIGNALED(childExitMethod) != 0) {
                                termSignal = WTERMSIG(childExitMethod);
                                printf("Foreground process %d terminated"
                                " by signal %d\n", childPid, termSignal);
                            }
                        } else {

                            printf("Background process %d beginning\n", spawnpid);
                            openJobs[numOpen] = spawnpid;
                            ++numOpen;
                        }
                            
                        break;
                }
            }
        }
    }

	return 0;
}

