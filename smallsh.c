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

/* struct to store connection info
struct Connection {
	int id;
	char connectName[10]; // stores names of connected rooms
};

// struct to store rooms info before writing to files
struct Room {
	char roomName[10];
	int id; // room number
	struct Connection connects[6];
	int numConnections;
};
*/

/*****************************************************************************
* createRoomsDir creates the directory with the process id and returns the
* directory's name
***************************************************************************** 

// create rooms directory 
char * createRoomsDir() {
	
	// get pid and convert;
	int pid = getpid();
	char sPid[6];
	sprintf(sPid, "%d", pid);

	// build directory name
	char *dir = "bernsa.rooms.";
	static char fullDir[19];
	memset(fullDir, '\0', sizeof(fullDir));
	strcpy(fullDir, dir);
	strcat(fullDir, sPid);

	// create rooms directory
	mkdir(fullDir, 0700);

	return fullDir;
}
*/

  
int main()
{

    char *command = NULL;
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

    // get user command
    while (1) {
        printf(":");
        charsEntered = getline(&command, &buffersize, stdin);

        // remove line feed
        command[charsEntered-1] = '\0';
        
        // check for and run built in commands
        if (strcmp(command, "exit") == 0) {
            
            // kill all processes, help from ibm.com
            kill(0, 15);;

            break;
        }

        if (strcmp(command, "status") == 0) {
            
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
    }

    // free memory
    free(command);



    /*
	char *roomsDir = createRoomsDir();

	char fileName[30];
	memset(fileName, '\0', sizeof(fileName));
	char fileNames[NUMROOMS][50]; // array of full path and file names

	char rName[15];
	char connection[30];
	char rType[30];
	size_t nwritten;
	int file_descriptor;
	memset(connection, '\0', sizeof(connection));
	memset(rType, '\0', sizeof(rType));

	// create each file and save room info
	for (i = 0; i < NUMROOMS; ++i) {
		file_descriptor = open(fileNames[i], O_WRONLY | O_CREAT, 0600);
		if (file_descriptor < 0) {
			fprintf(stderr, "Could not open %s\n", fileNames[i]);
			perror("Error in main()");
			exit(1);
		}
	
		// write room name
		strcpy(rName, "ROOM NAME: ");
		strcat(rName, roomArray[i].roomName);
		strcat(rName, "\n");
		nwritten = write(file_descriptor, rName, strlen(rName) * sizeof(char));
    */

	return 0;
}

