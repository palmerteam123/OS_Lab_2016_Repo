/***************************************************************
Operating Systems Lab Assignment 5 : MULTIPLE PRODUCERS AND CONSUMERS

Name : Kumar Ayush
Roll No. 13CS10058

Name : Varun Rawal
Roll No. 13CS10059

****************************************************************/

// Producer Program

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

#define MAX_SIZE 4

#define MATRIX_FILENAME "matrix.txt"

#define NUM_PRODUCERS 5
#define NUM_CONSUMERS 5
#define NUM_QUEUES 2

#define SEMAPHORE_KEY 10

// these are the indices of the sub-semaphores to be used as different mutex locks
#define MATRIX_MUTEX 0
#define QUEUE_0_MUTEX 1
#define QUEUE_1_MUTEX 2
#define EMPTY_SLOT_COUNTER_Q0 3
#define FULL_SLOT_COUNTER_Q0 4

#define EMPTY_SLOT_COUNTER_Q1 5
#define FULL_SLOT_COUNTER_Q1 6

#define CONSUMER_RECEIVE_CODE 100

#define MIN_RESOURCE_ACQUIRE_TIME 1 // in seconds

int semID;
int mssgQ0_ID,mssgQ1_ID;
int matrix_index;
int insert_counter = 0;

struct mssg // message template for the message Queue
{
	long mtype;
	char mtext[MAX_SIZE];
};

int MAX_SLOTS = 16384 / (sizeof(struct mssg)); // this is the initial value for the FULL_SLOT_COUNTER sub-semaphore of the semaphore array

// semaphore concepts :

// A semaphore S = +1 i.e. it is available
// A semaphore S = 0 i.e. it is NOT available, and any processes wanting to use it will be blocked (forced to sleep) on it, until the semaphore is again AVAILABLE (+1)

// I want to use semaphore S -->  use S with sem-op = -1
// I want to make S available --> use S with sem-op = +1

void updateMatrixFile(int queue_index,int entry)
{
	/*
		The matrix will be in following format :

				P1 P2 P3 P4 P5 C1 C2 C3 C4 C5
		Q0 : 	-   -  -  -  -  -  -  -  -  - 
		Q1 :	-   -  -  -  -  -  -  -  -  - 
	*/
	int matrix[NUM_QUEUES][NUM_PRODUCERS+NUM_CONSUMERS];

	FILE *fp = fopen(MATRIX_FILENAME, "r+");

	printf("\n\n");

	int i,j;
	for(i=0;i< NUM_QUEUES ;i++)
	{
		for(j=0;j< NUM_PRODUCERS ;j++)
		{
			fscanf(fp, "%d", &matrix[i][j]);
		}

		for(j=0;j< NUM_CONSUMERS ;j++)
		{
			fscanf(fp, "%d", &matrix[i][NUM_PRODUCERS+j]);
		}

		//char _n;
		//fscanf(fp, "%c", &_n); // for consuming \n
	}
/*
	for(i=0;i<NUM_QUEUES;i++)
	{
		for(j=0;j<10;j++)
			printf("%d ",matrix[i][j] );
		printf("\n");
	}
*/
	fclose(fp);

	//int myindex = getpid() - getppid() - 1;
	//printf("Before : %d\n",matrix[queue_index][matrix_index] );
	matrix[queue_index][matrix_index] = entry;	
	//printf("After : %d\n",matrix[queue_index][matrix_index] );

	//printf("my index : %d \n",myindex );
	//printf("matrix index : %d\n",matrix_index );

	
/*
	printf("\nchanged to\n\n");

	for(i=0;i<NUM_QUEUES;i++)
	{
		for(j=0;j<10;j++)
			printf("%d ",matrix[i][j] );
		printf("\n");
	}
*/
	// reading till now... now writing

	fp = fopen(MATRIX_FILENAME, "w+");

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

void writeCountertofile()
{
	char filename[50];
	strcpy(filename,"insert_temp_");

	char index_str[5];
	sprintf(index_str,"%d",matrix_index);

	strcat(filename,index_str);
	strcat(filename,".txt");

	FILE* fp = fopen(filename, "w+");
	fprintf(fp, "%d", insert_counter);	
	fclose(fp);
}

void acquireMutexLock(int sub_sema_index)
{
	struct sembuf sem_operator;

	sem_operator.sem_num = sub_sema_index;
	sem_operator.sem_op = -1;
	sem_operator.sem_flg = 0; // blocking wait, if mutex already acquired

	semop(semID,&sem_operator,1);
}

void releaseMutexLock(int sub_sema_index)
{
	struct sembuf sem_operator;

	sem_operator.sem_num = sub_sema_index;
	sem_operator.sem_op = +1;
	sem_operator.sem_flg = 0;

	semop(semID,&sem_operator,1);
}

void down(int sub_sema_index)
{
	struct sembuf sem_operator;

	sem_operator.sem_num = sub_sema_index;
	sem_operator.sem_op = -1;
	sem_operator.sem_flg = 0; // blocking wait, if already 0

	semop(semID,&sem_operator,1);
}

void up(int sub_sema_index)
{
	struct sembuf sem_operator;

	sem_operator.sem_num = sub_sema_index;
	sem_operator.sem_op = +1;
	sem_operator.sem_flg = 0;

	semop(semID,&sem_operator,1);
}

void produceIntoQueue(int insert,int queue_index)
{
	// trying to acquire lock
	// update the Matrix File accordingly
	acquireMutexLock(MATRIX_MUTEX);
	updateMatrixFile(queue_index,1); // entry should be either 0,1 or 2
	releaseMutexLock(MATRIX_MUTEX);

	// wait for an empty slot if none exists
	int EMPTY_SLOT_COUNTER = queue_index==0 ? EMPTY_SLOT_COUNTER_Q0 : EMPTY_SLOT_COUNTER_Q1;
	down(EMPTY_SLOT_COUNTER);	

	int mssgQ_ID = queue_index==0 ? mssgQ0_ID : mssgQ1_ID;

	if(queue_index==0)
		acquireMutexLock(QUEUE_0_MUTEX);
	else if(queue_index==1)
		acquireMutexLock(QUEUE_1_MUTEX);
	else
	{
		printf("Impossible Error !\n");
		exit(1);
	}

	// reached here --> lock acquired
	// update the Matrix File accordingly
	acquireMutexLock(MATRIX_MUTEX);
	updateMatrixFile(queue_index,2); // entry should be either 0,1 or 2
	releaseMutexLock(MATRIX_MUTEX);






	// producing a message item into the given Message Queue
	struct mssg buffer;

	strcmp(buffer.mtext,"");
	sprintf(buffer.mtext,"%d",insert);
	buffer.mtype=CONSUMER_RECEIVE_CODE;
	int retv = msgsnd(mssgQ_ID,&buffer,sizeof(buffer),0);	// blocking send

	if(retv == -1)
	{
		perror("Error : Message Queue !\n");
		exit(1);
	}
	else printf("PID %d succesfully inserted %d into Queue %d\n",getpid(),insert,queue_index);
	insert_counter++;
	writeCountertofile();

	
	sleep(MIN_RESOURCE_ACQUIRE_TIME);	// JUST for simulating Deadlock Conditions by prolonging the time of resource acquire




	if(queue_index==0)
		releaseMutexLock(QUEUE_0_MUTEX);
	else if(queue_index==1)
		releaseMutexLock(QUEUE_1_MUTEX);
	else
	{
		printf("Impossible Error !\n");
		exit(1);
	}

	// lock released 
	// update the Matrix File accordingly
	acquireMutexLock(MATRIX_MUTEX);
	updateMatrixFile(queue_index,0); // entry should be either 0,1 or 2
	releaseMutexLock(MATRIX_MUTEX);


	int FULL_SLOT_COUNTER = queue_index==0 ? FULL_SLOT_COUNTER_Q0 : FULL_SLOT_COUNTER_Q1;
	up(FULL_SLOT_COUNTER);

	
}


void main(int argc,char* argv[])
{
	semID = semget((key_t)SEMAPHORE_KEY,7,IPC_CREAT|0666);

	int MSSG_Q0_KEY,MSSG_Q1_KEY;
	
	if(argc!=4)
	{
		printf("Please run the program with 3 arguments : <MSSG_Q0_KEY> <MSSG_Q1_KEY> <matrix_index> \n");
		exit(1);
	}
	else
	{
		sscanf(argv[1],"%d",&MSSG_Q0_KEY);
		sscanf(argv[2],"%d",&MSSG_Q1_KEY);
		sscanf(argv[3],"%d",&matrix_index);
	//	sscanf(argv[3],"%f",&sleep_prob);
	}

	mssgQ0_ID=msgget((key_t)MSSG_Q0_KEY,IPC_CREAT|0666);
	mssgQ1_ID=msgget((key_t)MSSG_Q1_KEY,IPC_CREAT|0666);

	srand(time(NULL));

	while(1)
	{
		int random_queue = rand()%2;

		int randomizer = rand()%200;
		if(randomizer>100)
			random_queue = 1 - random_queue;

		int random_number = rand()%50 + 1; // to be inserted into the random random_queue

		printf("PID %d trying to insert %d into Queue %d\n",getpid(),random_number,random_queue );
		
		produceIntoQueue(random_number,random_queue);

		int random_sleep_time = rand()%5 + 2; // sleeps randomly for 2 - 6 seconds

		sleep(random_sleep_time);

	}
}