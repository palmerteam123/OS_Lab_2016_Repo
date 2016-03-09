/***************************************************************
Operating Systems Lab Assignment 5 : MULTIPLE PRODUCERS AND CONSUMERS

Name : Kumar Ayush
Roll No. 13CS10058

Name : Varun Rawal
Roll No. 13CS10059

****************************************************************/

// Manager Program : creates 5 Producer processes and 5 Consumer Processes

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/sem.h>
/*

INITIALIZATION:

	shared binary semaphore mutex = 1;
	shared counting semaphore empty = MAX;
	shared counting semaphore full = 0;
	shared anytype buffer[MAX];
	shared int in, out, count;


PRODUCER :

	anytype item;
	
	// here, P() --> sleepOn() /wait()  and V() --> release() / signal()

	repeat {

		// produce something 
		item = produce();

		// wait for an empty space 
		P(empty);

		// store the item 
		P(mutex);
		buffer[in] = item;
		in = in + 1 mod MAX;
		count = count + 1;
		V(mutex);

		// report the new full slot 
		V(full);

	} until done;




CONSUMER:

	anytype item;

	repeat {

		// wait for a stored item
		P(full);

		// remove the item 
		P(mutex);
		item = buffer[out];
		out = out + 1 mod MAX;
		count = count - 1;
		V(mutex);

		// report the new empty slot 
		V(empty);

		// consume it 
		consume(item);

	} until done;

	*/

// IMPLEMENTATION using LINUX/POSIX Sempahores

// the "shared semaphore" word in the above theory is implemented by extracting the sempahores array using a common key_t among different using processes 
// the "shared buffer and items" word in the above theory is implemented by extracting the message queue using a common key_t among different using processes 
// also a record file "matrix.txt" is a shared one

// So, 3 mutex locks required : one for each of the Queues, one for the matrix.txt file

// semaphore concepts :

// A semaphore S = +1 i.e. it is available
// A semaphore S = 0 i.e. it is NOT available, and any processes wanting to use it will be blocked (forced to sleep) on it, until the semaphore is again AVAILABLE (+1)

// I want to use semaphore S -->  use S with sem-op = -1
// I want to make S available --> use S with sem-op = +1

#define MATRIX_FILENAME "matrix.txt"
#define NUM_PRODUCERS 5
#define NUM_CONSUMERS 5
#define NUM_QUEUES 2
#define MSSG_Q0_KEY 10
#define MSSG_Q1_KEY 20

#define SEMAPHORE_KEY 10

// these are the indices of the sub-semaphores to be used as different mutex locks
#define MATRIX_MUTEX 0
#define QUEUE_0_MUTEX 1
#define QUEUE_1_MUTEX 2
#define EMPTY_SLOT_COUNTER_Q0 3
#define FULL_SLOT_COUNTER_Q0 4

#define EMPTY_SLOT_COUNTER_Q1 5
#define FULL_SLOT_COUNTER_Q1 6

#define MAX_SIZE 4

int semID;

struct mssg // message template for the message Queue
{
	long mtype;
	char mtext[MAX_SIZE];
};

int MAX_SLOTS = 16384 / (sizeof(struct mssg));

void initializeAllSubSemaphoreValues()
{
	semctl(semID,MATRIX_MUTEX,SETVAL,1);
	semctl(semID,QUEUE_0_MUTEX,SETVAL,1);
	semctl(semID,QUEUE_1_MUTEX,SETVAL,1);

	semctl(semID,EMPTY_SLOT_COUNTER_Q0,SETVAL,MAX_SLOTS);
	semctl(semID,FULL_SLOT_COUNTER_Q0,SETVAL,0);

	semctl(semID,EMPTY_SLOT_COUNTER_Q1,SETVAL,MAX_SLOTS);
	semctl(semID,FULL_SLOT_COUNTER_Q1,SETVAL,0);
}
//semctl(semID,0,GETVAL,0);

void initializeMatrixFile()
{
	/*
		The matrix will be in following format :

				P1 P2 P3 P4 P5 C1 C2 C3 C4 C5
		Q0 : 	-   -  -  -  -  -  -  -  -  - 
		Q1 :	-   -  -  -  -  -  -  -  -  - 
	*/

	int matrix[NUM_QUEUES][NUM_PRODUCERS+NUM_CONSUMERS];

	int i,j;

	for(i=0;i<NUM_QUEUES;i++)
	{
		for(j=0;j<NUM_PRODUCERS + NUM_CONSUMERS;j++)
			matrix[i][j] = 0;
//		printf("\n");
	}

	FILE *fp = fopen(MATRIX_FILENAME, "w+");

	for(i=0;i< NUM_QUEUES ;i++)
	{
		for(j=0;j< NUM_PRODUCERS ;j++)
		{
			fprintf(fp, "%d ", matrix[i][j]);
		}

		for(j=0;j< NUM_CONSUMERS ;j++)
		{
			fprintf(fp, "%d ", matrix[i][NUM_PRODUCERS+j]);
		}

		fprintf(fp, "\n");
	}

	fclose(fp);

}

void main(int argc,char* argv[])
{
	int DPP;

	if(argc!=2)
	{
		printf("Please run the program with 1 argument : <DPP_toggle (0 or 1)> \n"); // DPP --> Deadlock Prevention Protocol
		exit(1);
	}
	else
	{
		sscanf(argv[1],"%d",&DPP);
		if(DPP != 0 && DPP != 1)
		{
			printf("Please run the program with 1 argument : <DPP_toggle (0 or 1)> \n"); // DPP --> Deadlock Prevention Protocol
			exit(1);
		}
	}

	semID = semget((key_t)SEMAPHORE_KEY,7,IPC_CREAT|0666);

	initializeAllSubSemaphoreValues();
	initializeMatrixFile();

	float probability = 0.2;

	int mssgQ0_ID=msgget((key_t)MSSG_Q0_KEY,IPC_CREAT|0666);
	int mssgQ1_ID=msgget((key_t)MSSG_Q1_KEY,IPC_CREAT|0666);

	int i;
	for(i=0;i<NUM_PRODUCERS;i++)
	{
		char command[50];
		sprintf(command,"xterm -hold -e ./producer %d %d %d &",MSSG_Q0_KEY,MSSG_Q1_KEY,i);
		system(command);
		sleep(1);	// otherwise the item to be first inserted becomes the same for all the Queues due to the timeseeding : srand(time(NULL)) 
	}
/*
	for(i=0;i<NUM_CONSUMERS;i++)
	{
		char command[50];
		sprintf(command,"xterm -hold -e ./consumer %d %d %d %f %d &",MSSG_Q0_KEY,MSSG_Q1_KEY,NUM_PRODUCERS+i,probability,DPP);
		system(command);
	}
*/

}