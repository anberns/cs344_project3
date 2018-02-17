/*****************************************************************************
* bernsa.buildrooms.c
* Aaron Berns
* 2/5/18
*
* Creates a directory to store room files, creates room files and saves them 
* to directory.
*****************************************************************************/ 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define NUMROOMS 7

// struct to store connection info
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

/*****************************************************************************
* createRoomsDir creates the directory with the process id and returns the
* directory's name
*****************************************************************************/ 

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

/*****************************************************************************
* randomInt returns an integer within the specified range, inclusive
*****************************************************************************/ 

int randomInt(int min, int max) {
	return rand() % (max + 1 - min) + min;
}	

/*****************************************************************************
* makeConnections is passed the address of an array of Room structs, which
* is already filled with each room's id, name and type. It randomly makes
* connections between rooms up to a total of 6 per room, making sure that
* the connections are two way.
*****************************************************************************/ 

// connect rooms
void makeConnections(struct Room *sArray) {
	int i, j, badMatch = 0; 
	
	// initialize sArray
	for (i = 0; i < NUMROOMS; ++i) {
		for (j = 0; j < NUMROOMS -1; ++j) {
			sArray[i].connects[j].id= -1;
			memset(sArray[i].connects[j].connectName, '\0', sizeof(sArray[i].connects[j].connectName));
		}
		sArray[i].numConnections = 0;
	}

	// make array of room ids that were chosen
	// this helps when checking if a random room number
	// was actually chosen to be a partipating room
	int ids[NUMROOMS];
	for (i = 0; i < NUMROOMS; ++i) {
		ids[i] = sArray[i].id;
	}
	
	// get two random indexes 	
	int a, b;
	int aFlag = 0; // set to 1 if chosen id is a good choice
	int bFlag = 0;
	int aIndex, bIndex;
	i = 0;	
	do {
		a = randomInt(0, 9);
		b = randomInt(0, 9);	

		// check that a and b reference room numbers that were chosen out of
		// possible 10, save index at which that room number is stored in
		// the Room array, which is different from it's id
		for (j = 0; j < NUMROOMS; ++j) {
			if (ids[j] == a) {
				aFlag = 1;
				aIndex = j;
			}
			if (ids[j] == b) {
				bFlag = 1;
				bIndex = j;
			}
		}

		// check for a possible connection to self
		// reset flags if so
		if (aFlag && sArray[aIndex].id == b) {
			aFlag = 0;
		}
		if (bFlag && sArray[bIndex].id == a) {
			bFlag = 0;
		}

		// continue with matching process if choices are good
		if (aFlag == 1 && bFlag == 1) {

			// make sure each choice has an available connection
			// if not, increment total matches and chose a and b again
			if (sArray[aIndex].numConnections == 6 || sArray[bIndex].numConnections == 6) {
				++i;

			} else {

				for (j = 0; j < NUMROOMS - 1; ++j) {
					
					// check for already connected
					if (sArray[aIndex].connects[j].id == b) {
						badMatch = 1;
						break;
					}

				}

				// make match if all criteria are met
				if (badMatch== 0) {
					sArray[aIndex].connects[sArray[aIndex].numConnections].id = b;
					++sArray[aIndex].numConnections;
					sArray[bIndex].connects[sArray[bIndex].numConnections].id = a;
					++sArray[bIndex].numConnections;

					++i;
				}
				badMatch = 0;
			}
		}
		aFlag = 0;
		bFlag = 0;

	// continue until selected number of matches are made, roughly the number of rooms * connections
	// adjusting this value adjusts the average number of connections per room
	} while (i < 39);
}
   
int main()
{
	// seed random generator
	time_t t;
	srand((unsigned) time(&t));

	// create rooms dir and save path
	char *roomsDir = createRoomsDir();

	// array of room names
	const char *possNames[10];
	possNames[0] = "Respite";
	possNames[1] = "Dark";
	possNames[2] = "Shadow";
	possNames[3] = "Twilight";
	possNames[4] = "Limbo";
	possNames[5] = "Sacred";
	possNames[6] = "Termina";
	possNames[7] = "Silent";
	possNames[8] = "Haze";
	possNames[9] = "Tangent";

	// get 7 random unique index numbers
	int ranIndex[NUMROOMS]; 
	int i, j, possIndex, repeatFlag = 0;

	// initialize ranIndex
	for (i = 0; i < NUMROOMS; ++i) {
		ranIndex[i] = -1;
	}

	// load ranIndex with random numbers
	i = 0;
	do {
		possIndex = randomInt(0, 9);
		for (j = 0; j <= i; ++j) {
			if (ranIndex[j] == possIndex) {
				repeatFlag = 1;
				break;
			}	
		}
		if (repeatFlag == 0) {
			ranIndex[i] = possIndex;
			++i;
		}
		repeatFlag = 0;
	} while (i < NUMROOMS);
		 
	// create array of file names
	const char *fNames[NUMROOMS];
	fNames[0] = "file0";
	fNames[1] = "file1";
	fNames[2] = "file2";
	fNames[3] = "file3";
	fNames[4] = "file4";
	fNames[5] = "file5";
	fNames[6] = "file6";

	char fileName[30];
	memset(fileName, '\0', sizeof(fileName));
	char fileNames[NUMROOMS][50]; // array of full path and file names

	// save all full file names
	for (i = 0; i < NUMROOMS; ++i) {
		strcpy(fileName, roomsDir);
		strcat(fileName, "/");
		strcat(fileName, fNames[i]);
		strcpy(fileNames[i], fileName);
		memset(fileName, '\0', sizeof(fileName));	  
	}
	
	// add room names and ids to structs
	struct Room roomArray[7];
	for (i = 0; i < NUMROOMS; ++i) {
		memset(roomArray[i].roomName, '\0', sizeof(roomArray[i].roomName));
		strcpy(roomArray[i].roomName, possNames[ranIndex[i]]);
		roomArray[i].id = ranIndex[i];
	}

	// make connections and store in struct array
	struct Room* rArrP = roomArray;
	makeConnections(rArrP);

	// add room names to connections
	for (i = 0; i < NUMROOMS; ++i) {
		for (j = 0; j < NUMROOMS -1; ++j) {
			if (roomArray[i].connects[j].id != -1) {
				strcpy(roomArray[i].connects[j].connectName, possNames[roomArray[i].connects[j].id]);
			}
		}
	}
	
	// create files and write to files
	const char *rTypes[NUMROOMS+1];
	rTypes[0] = "?"; 	// not sure why but index 0 is empty after assignment
	rTypes[1] = "START_ROOM";
	rTypes[2] = "MID_ROOM";
	rTypes[3] = "MID_ROOM";
	rTypes[4] = "MID_ROOM";
	rTypes[5] = "MID_ROOM";
	rTypes[6] = "MID_ROOM";
	rTypes[7] = "END_ROOM";

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

		// write connections
		j = 0;
		int connectCount = 0;
		char iStr[2];
		while (roomArray[i].connects[j].id != -1 && connectCount < NUMROOMS -1 ) {
			strcpy(connection, "CONNECTION ");
			sprintf(iStr, "%d: ", j+1);
			strcat(connection, iStr);
			strcat(connection, roomArray[i].connects[j].connectName);
			strcat(connection, "\n");
			nwritten = write(file_descriptor, connection, strlen(connection) * sizeof(char));
			memset(connection, '\0', sizeof(connection));
			++j;
			++connectCount;
		}
		
		// write room type
		strcpy(rType, "ROOM TYPE: ");
		strcat(rType, rTypes[i+1]);
		strcat(rType, "\n");
		nwritten = write(file_descriptor, rType, strlen(rType) * sizeof(char));

		// clear output strings and close file
		memset(rType, '\0', sizeof(rType));
		memset(rName, '\0', sizeof(rName));
		close(file_descriptor); 
		connectCount = 0;
	}

	return 0;
}

