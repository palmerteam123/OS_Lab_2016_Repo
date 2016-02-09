/***************************************************************
Operating Systems Lab Assignment 1 : Problem 1

Name : Kumar Ayush
Roll No. 13CS10058

Name : Varun Rawal
Roll No. 13CS10059

****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/ipc.h>

#include <signal.h>// THIS PROGRAM HAS BEEN IMPLEMENTED USING INTER-PROCESS COMMUNICATION THROUGH SIGNALS 

int* arr,search_count=0;	// global array of integers (received as input from the file)

int main_PID;	// globally available Process ID of the main process

void dls(int*,int,int,int);

void signalSender(int n)
{
	union sigval obj;
	obj.sival_int=n;

	//printf("sending SIGINT signal to %d......\n",pid);

   	if(sigqueue(main_PID,SIGINT,obj) == -1)
   	{
    	perror("Signal NOT SENT...sigqueue error ! Terminating...");
    	exit(EXIT_FAILURE);
   	}			
}

void signalHandler(int signo, siginfo_t *info,void *ctx)
{
    printf("receive the data from siqueue by info->si_value.sival_int is %d\n",info->si_value.sival_int);
    search_count++;
}



int main(int argc,char* argv[])
{
	main_PID=getpid();

	if(argc!=3)
	{
		printf("Sorry ! Invalid argument count ! Please provide the input file name and the search integer as arguments...\n");
		exit(1);
	}

	FILE* _inp_=fopen(argv[1],"r");

	if(_inp_ == NULL)
	{
		printf("Cannot open file ! Terminating...\n");
		exit(1);
	}

	int i,num,count=0;

	while(fscanf(_inp_,"%d",&num) !=  EOF)
		count++;

	arr=(int*)malloc(count*sizeof(int));
	rewind(_inp_);

	for(i=0;i<count;i++)
		fscanf(_inp_,"%d",&arr[i]);

	dls(arr,atoi(argv[2]),0,count-1);

	int temp=0;

	struct sigaction act;
    act.sa_sigaction = signalHandler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;//Information transfer switch
    if(sigaction(SIGINT,&act,NULL) == -1){
        perror("sigaction error");
        exit(EXIT_FAILURE);
    }
  	
  	pause();
    
	
	fclose(_inp_);
		
}


void dls(int* A,int N,int beg,int end)
{
	if(end-beg+1 <=5)
	{
		int k;
		for(k=beg;k<=end;k++)
			if(A[k]==N)
			{
				//printf("I found @ position : %d\n", k);
				signalSender(k);
			}
		return;
	}
	else
	{
		int mid=(beg+end)/2;

		int fork_val1=fork();
		
		if(fork_val1<0)
		{
			printf("Sorry ! Out of memory resources ! Cannot continue the search. Terminating ....\n");
			exit(1);
		}

		if(fork_val1==0)
			{
				dls(A,N,beg,mid);
				exit(0);
			}


		int fork_val2=fork();

		if(fork_val2<0)
		{
			printf("Sorry ! Out of memory resources ! Cannot continue the search. Terminating ....\n");
			exit(1);
		}

		if(fork_val2==0)
		{
			dls(A,N,mid+1,end);
			exit(0);
		}
		
		// wait for both the children to terminate
		return;
	
	}
}
