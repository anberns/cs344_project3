/*****************************************************************************
* bernsa.adventure.c
* Aaron Berns
* 2/2/18
*
* Help with mutex and pthread setup from thegeekstuff.com
* Help with strftime from tutorialspoint.com
*****************************************************************************/ 

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h>

#define NUMROOMS 7
#define PATHSIZE 20

pthread_t threadID[2];
pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;

// struct to store connection info
struct Connection {
	int id;
	char connectName[10];
};

// struct to store rooms connections before writing to files
struct Room {
	char roomName[10];
	char roomType[10];
	struct Connection connects[6];
	int numConnections;
};

/******************************************************************************
*  newestDir is passed the address of the string to store the newest directory
*  name, finds it and saves it
*  Based on block 2 lectures and readings 
******************************************************************************/
void newestDir(char *newestDirName) {
	int newestDirTime = -1;
	char targetDirPrefix[32] = "bernsa.rooms.";
	memset(newestDirName, '\0', sizeof(newestDirName));

	DIR* dirToCheck;
	struct dirent *fileInDir;
	struct stat dirAttributes;

	dirToCheck = opendir(".");

	if (dirToCheck > 0) {
		while ((fileInDir = readdir(dirToCheck)) != NULL) {
			if (strstr(fileInDir->d_name, targetDirPrefix) != NULL) {
				stat(fileInDir->d_name, &dirAttributes);

				if ((int)dirAttributes.st_mtime > newestDirTime) {
					newestDirTime = (int)dirAttributes.st_mtime;
					memset(newestDirName, '\0', sizeof(newestDirName));
					strcpy(newestDirName, fileInDir->d_name);
				}
			}
		}
	}

	closedir(dirToCheck);
}

/******************************************************************************
* initRoomArray is passed the address of an array of Room structs and 
* initializes the values 
******************************************************************************/

void initRoomArray(struct Room *rArray) {
	
	int i, j;
	for (i = 0; i < NUMROOMS; ++i) {
		for (j = 0; j < NUMROOMS -1; ++j) {

			// connection id indexes that remain -1 are not active connections
			rArray[i].connects[j].id= -1;
			memset(rArray[i].connects[j].connectName, '\0', sizeof(rArray[i].connects[j].connectName));
		}
		memset(rArray[i].roomName, '\0', sizeof(rArray[i].roomName));
		memset(rArray[i].roomType, '\0', sizeof(rArray[i].roomName));

		rArray[i].numConnections = 0;
	}

}

/******************************************************************************
* loadRoomArray is passed the address of an array of Room structs and the
* name of the directory where room files are stored. It opens each room file
* and saves the information into the Room array. It returns either 0 or the
* value of an error it encounters. 
******************************************************************************/

int loadRoomArray(struct Room *rArray, char *dirName) {

	DIR* dirToCheck;
	struct dirent *fileInDir;
	struct stat fileAttributes;
	char targetDirPrefix[5] = "file";
	char fullPath[50];
	FILE* curFile;;
	char tempString[50]; // stores a line of the file
	char parse1[15], parse2[15], parse3[15]; // stores sought after values from line
	
	int i, j;

	dirToCheck = opendir(dirName);
	if (dirToCheck > 0) {
		i = 0;
		while ((fileInDir = readdir(dirToCheck)) != NULL) {
			if (strstr(fileInDir->d_name, targetDirPrefix) != NULL) {
			
				// create path to file
				memset(fullPath, '\0', sizeof(fullPath));
				strcpy(fullPath, dirName);
				strcat(fullPath, "/");
				strcat(fullPath, fileInDir->d_name);

				// open file
				curFile = fopen(fullPath, "r");
				if (curFile == NULL) {
					perror("Error opening file");
					return(1);
				}

				// save contents
				j = 0;
				while (fgets(tempString, 50, curFile) != NULL) {

					// parse and save room data
					sscanf(tempString, "%s %s %*s", parse1, parse2);

					// determine data info type of line and save appropriately
					if (strcmp(parse2, "NAME:") == 0) {
						sscanf(tempString, "%*s %*s %s", parse3);
						strcpy(rArray[i].roomName, parse3);
					} else if (strcmp(parse2, "TYPE:") == 0) {
						sscanf(tempString, "%*s %*s %s", parse3);
						strcpy(rArray[i].roomType, parse3);
					} else if (strcmp(parse1, "CONNECTION") == 0) {
						sscanf(tempString, "%*s %*s %s", parse3);
						strcpy(rArray[i].connects[j].connectName, parse3);
						rArray[i].numConnections++;
						++j;
					}

				}

				fclose(curFile);
				++i;
			}
		}
	}
	
	closedir(dirToCheck);
	return 0;
}

/******************************************************************************
* findIndex is passed the address of an array of Room structs, the addresses
* of a strings storing a value to search for and the field to search in. It 
* returns the index or -1 if not found. 
******************************************************************************/

int findIndex(struct Room *rArray, char *searchTerm, char *searchField) {
	int i, j, index;

	// search is for the name of the room
	if (strcmp(searchField, "roomName") == 0) {
		for (i = 0; i < NUMROOMS; ++i) {
			if (strcmp(rArray[i].roomName, searchTerm) == 0) {
				index = i;
			}
		}

	// search is for the type of the room
	} else if (strcmp(searchField, "roomType") == 0) {
		for (i = 0; i < NUMROOMS; ++i) {
			if (strcmp(rArray[i].roomType, searchTerm) == 0) {
				index = i;
			}
		}

	// not found
	} else {
		index = -1;
	}	

	return index;
}

/******************************************************************************
* promptLocCons is passed the address of an array of Room structs and the
* index of a given room stored in the array. It displays the room name and
* all of its connections to the user.
******************************************************************************/

void promptLocCons(struct Room *rArray, int rIndex) {
	char locPrompt[50];
	char conPrompt[200];
	memset(locPrompt, '\0', sizeof(locPrompt));
	memset(conPrompt, '\0', sizeof(conPrompt));

	// display location
	strcpy(locPrompt, "CURRENT LOCATION: ");
	strcat(locPrompt, rArray[rIndex].roomName);
	strcat(locPrompt, "\n");
	printf("%s", locPrompt);

	// display connections
	strcpy(conPrompt, "POSSIBLE CONNECTIONS: ");
	int i, j;
	for (i = 0; i < rArray[rIndex].numConnections; ++i) {
		strcat(conPrompt, rArray[rIndex].connects[i].connectName);
		strcat(conPrompt, ", ");
	}

	// replace last ',' with '.'
	conPrompt[strlen(conPrompt) -2] = '.';
	printf("%s\n", conPrompt);

}

/******************************************************************************
* promptNext is passed the address of an array of Room structs, the address
* of a string storing the name of the room the player entered to go to next
* and the index of the player's current room stored in the array. It searches the 
* current room's connections for the user's choice, returning 1 if it is valid,
* 0 if not.
******************************************************************************/

int promptNext(struct Room *rArray, char *move, int cIndex) {
	
	// look for chosen move in connections
	int i; 
	for (i = 0; i < rArray[cIndex].numConnections; ++i) {

		// found
		if (strcmp(rArray[cIndex].connects[i].connectName, move) == 0) {
			return 1;
		}
	}	

	// not found
	return 0;
}
	
/******************************************************************************
* promptNext is passed the address of an array of Room structs, the address
* of an array of ints holding the index of the rooms in the player's path
* and the number of rooms the player has visited. It displays the game ending
* message with this information.
******************************************************************************/

void promptEnd(struct Room *rArrPtr, int *pathPtr, int steps) {
	printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n"); 
	printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", steps-1);
	int i;

	// print room names visited in order
	for (i = 1; i < steps; ++i) {
		printf("%s\n", rArrPtr[pathPtr[i]].roomName);
	}
}

/******************************************************************************
* getTime writes the current time to "currentTime.txt". Overwrites file on each
* call.  
******************************************************************************/

void * getTime() {

	// block runGame
	pthread_mutex_lock(&myMutex);

	time_t rawtime;
	struct tm *tInfo;
	char sTime[100];
	FILE* timeFile;
	char timeFileName[25];
	memset(timeFileName, '\0', sizeof(timeFileName));
	strcpy(timeFileName, "currentTime.txt");


	// get current time and store in specified format
	time (&rawtime);
	tInfo = localtime(&rawtime);
	strftime(sTime, 100, "%I:%M%p, %A, %B %d, %Y", tInfo);

	// write time string to time file
	timeFile = fopen(timeFileName, "w");
	fprintf(timeFile, "%s\n", sTime);
	fclose(timeFile);	

	// unblock runGame
	pthread_mutex_unlock(&myMutex);

	return NULL;
}

/******************************************************************************
* runGame is passed the address of the directory of rooms being used for the
* game. It is responsible for calling all of the above helper functions and
* doing everything else needed to lead the player through the rooms. It also
* create a new thread to run getTime() when the user enters "time".
******************************************************************************/

void* runGame(void *nDirPtr) {

	// block time thread
	pthread_mutex_lock(&myMutex);
	
	// create array of Room structs to store file info
	struct Room roomArray[NUMROOMS];
	struct Room *rArrPtr = roomArray; 

	// initialize array
	initRoomArray(rArrPtr);

	// load rooms from files
	int loadReturn = loadRoomArray(rArrPtr, nDirPtr);

	// get index of starting and ending rooms
	char searchTerm[10];
	char * sTermPtr = searchTerm;
	memset(searchTerm, '\0', sizeof(searchTerm));
	strcpy(searchTerm, "START_ROOM");
	char searchField[10];
	char * sFieldPtr = searchField;
	memset(searchField, '\0', sizeof(searchField));
	strcpy(searchField, "roomType");

	int startIndex = findIndex(rArrPtr, sTermPtr, sFieldPtr);

	memset(searchTerm, '\0', sizeof(searchTerm));
	strcpy(searchTerm, "END_ROOM");

	int endIndex = findIndex(rArrPtr, sTermPtr, sFieldPtr);

	// display and prompt starting room
	promptLocCons(rArrPtr, startIndex);
	int curIndex = startIndex;

	char *nextMove = NULL;
	int charsEnt, i;
	size_t bufferSize = 0;
	char moveNoLF[10]; // stores player input without LF

	// begin storing player's path 
	int path[PATHSIZE];
	int *pathPtr = path;
	for (i = 0; i < PATHSIZE; ++i) {
		path[i] = -1;
	}
	int j = 0;
		
	// create time file in current directory
	FILE* timeFile;
	char timeFileName[25];
	memset(timeFileName, '\0', sizeof(timeFileName));
	strcpy(timeFileName, "currentTime.txt");
	char *timeIn;
	int numChars;
	size_t bufSize = 0;

	// loop through until END_ROOM, create time thread when appropriate
	while (curIndex != endIndex) {

		// clear for each write and copy
		memset(moveNoLF, '\0', sizeof(moveNoLF));

		// prompt user for next move
		printf("WHERE TO? >");
		charsEnt = getline(&nextMove, &bufferSize, stdin);
	
		// remove line feed from input so strcmp works
		i = 0;
		while (nextMove[i] != 10) {
			moveNoLF[i] = nextMove[i];
			++i;
		}

		int result_code_time;

		// check for and run time thread
		if (strcmp(moveNoLF, "time") == 0) {

			// unblock time thread
			pthread_mutex_unlock(&myMutex);

			// start time thread
			result_code_time = pthread_create(&threadID[1], NULL, getTime, NULL);
			pthread_join(threadID[1], NULL);

			// take back control
			pthread_mutex_lock(&myMutex);
		
			// read time file and print
			timeFile = fopen(timeFileName, "r");
			numChars = getline(&timeIn, &bufSize, timeFile);
			printf("\n%s\n", timeIn);
			fclose(timeFile);
			
		}

		// check for room input errors, proceed if none
		else if (promptNext(rArrPtr, moveNoLF, curIndex) == 1) {
			memset(searchTerm, '\0', sizeof(searchTerm));
			strcpy(searchTerm, moveNoLF);
			memset(searchField, '\0', sizeof(searchField));
			strcpy(searchField, "roomName");
			path[j] = curIndex;
			curIndex = findIndex(rArrPtr, sTermPtr, sFieldPtr);
			printf("\n");
			if (curIndex != endIndex) {
				promptLocCons(rArrPtr, curIndex);
			}
			++j;
		} 	

		// input error, prompt to try again
		else {
			printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
			promptLocCons(rArrPtr, curIndex);
		}

	}		

	// add end room to path
	path[j] = endIndex;

	// display ending message and path
	promptEnd(rArrPtr, pathPtr, j+1);
	
	// free memory from getline use
	free(nextMove);
	free(timeIn);

	// remove lock before exiting game
	pthread_mutex_unlock(&myMutex);

	return NULL;
}

int main() 
{
	// get newest rooms directory name
	char newestDirName[256];
	char *nDirPtr = newestDirName;
	newestDir(nDirPtr);

	// initialize lock
	pthread_mutex_init(&myMutex, NULL);

	// start game thread
	int result_code_game = pthread_create(&threadID[0], NULL, runGame, nDirPtr);
	
	// allow threads to block each other
	pthread_join(threadID[0], NULL);
	
	// destroy lock
	pthread_mutex_destroy(&myMutex);

	nDirPtr = NULL;

	return 0;
}
