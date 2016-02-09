/***************************************************************
Operating Systems Lab Assignment 1 : Problem 2

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
#include <string.h>
#include <sys/stat.h>

#define MAX_SIZE 1024

char history_file_path[MAX_SIZE];

int clrscreen()
{
	system("clear");
	return 1;
}

int showEnvironmentVariables()
{
	system("env");
	return 1;
}

int showWorkingDirectory()
{
	char cwd[MAX_SIZE];

	if(getcwd(cwd, MAX_SIZE) != NULL)
		printf("%s\n",cwd);
		
	else
	{
		printf("Command Prompt error ! \n ");
		return -1;
	}

	return 1;
}

int updateHistory(char* s)
{
	FILE* _out_=fopen(history_file_path,"a+");

	if(_out_ == NULL)
	{
		printf("Cannot update history ! \n");
		return -1;
	}

	char sent[MAX_SIZE];
	int t=0;

	while(fgets(sent, MAX_SIZE, _out_)!=NULL)
	{
		char *temp=strtok(sent," \t\n");
		t=atoi(temp);
	}

	fprintf(_out_, "%d %s",t+1,s );
	fclose(_out_);

	return 1;
}

int changeDirectory(char* s)
{
	char cwd[MAX_SIZE];
	{
		if(s==NULL)
			chdir("/home");
		else
			{
				if(getcwd(cwd, MAX_SIZE) != NULL && strcmp(cwd,"/home")==0);
				else return chdir(s);
			}
		}

	return 1;
}

int makeDirectory(char* s)
{
	return mkdir(s,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

int removeDirectory(char* s)
{
	return rmdir(s);
}

int listContents(char* s)
{
	
	if(s==NULL)
		system("ls");
	else if(strcmp(s,"-l")==0)
		system("ls -l");
	else
		printf("Sorry ! This shell supports only the following \n\n ls\nls -l\n");

	return 1;
}

int showHistory(char* s)
{

	if(s==NULL)
	{

		FILE* _inp_=fopen(history_file_path,"r");

		if(_inp_ == NULL)
		{
			printf("Cannot open file ! Terminating...\n");
			return -1;
		}

		char sent[MAX_SIZE];
				
		while(fgets(sent, MAX_SIZE, _inp_)!=NULL)
			printf("%s",sent);
		
		fclose(_inp_);
	}

	else
	{
		int hist_arg=atoi(s);

		if(hist_arg==0)
		{
			printf("Please enter an integer argument as : history <int_arg>\n");
			return -1;
		}
		else
			{

				FILE* _inp_=fopen(history_file_path,"r");

				if(_inp_ == NULL)
				{
					printf("Cannot open file ! Terminating...\n");
					return -1;
				}

				char sent[MAX_SIZE];
				int count=0,tot_count=0;
				
				while(fgets(sent, MAX_SIZE, _inp_)!=NULL)tot_count++;

				rewind(_inp_);
				while(fgets(sent, MAX_SIZE, _inp_)!=NULL)
				{
					count++;
					if(count>tot_count - hist_arg)
						printf("%s",sent);
				}

				fclose(_inp_);

				}
			}
	return 1;
}

void executeCommand(char** s)
{

    int pid = fork();
    int ret;

	if(pid == 0) 
		{
			ret=execvp(s[0],s);
			printf("Invalid command !\n");
			exit(0);
		}
	

	int count=0;
	while(s[count]!=NULL)
		count++;

	if(strcmp(s[count-1],"&")==0) ;
	else wait(NULL);

}

void loop_SHELL()
{
	FILE* _io_=fopen("commands.txt","a+");
	
	realpath("commands.txt", history_file_path);
	
   	while(1)
	{
		char cwd[MAX_SIZE];
		char buf[MAX_SIZE];

		FILE* _out_;

		if(getcwd(cwd, MAX_SIZE) != NULL)
		printf(" %s > ",cwd);
		
		else{
			printf("Command Prompt error ! Terminating...\n ");
			exit(1);
		}

		if(fgets(buf, MAX_SIZE, stdin)==NULL)
			continue;

		if(strlen(buf)<=1)
			continue;

		if(updateHistory(buf)==-1)
			printf("Trouble updating History !\n");

		char **args = (char**)malloc(MAX_SIZE*sizeof(char*));

		char** p =args;

		*p=strtok(buf, " \t\n");

		while(*p!=NULL)
		{
			p++;
			*p=strtok(NULL, " \t\n");
		}

		if(args==NULL)
			continue;

		if(strcmp(args[0],"clear")==0 && args[1]==NULL)
			clrscreen();

		else if(strcmp(args[0],"ls")==0)
			listContents(args[1]);

		else if(strcmp(args[0],"env")==0 && args[1]==NULL)
			showEnvironmentVariables();

		else if(strcmp(args[0],"exit")==0 && args[1]==NULL)
			exit(0);

		else if(strcmp(args[0],"pwd")==0 && args[1]==NULL)
		{
			if (showWorkingDirectory() == -1)
				printf("Error showing current directory !\n");
		}

		else if(strcmp(args[0],"cd")==0)
		{
			if(changeDirectory(args[1])==-1)
				printf("ERROR : No such directory exists!\n");
		}
		
		else if(strcmp(args[0],"mkdir")==0)
		{
			if(args[1]==NULL)
				printf("Invalid Command ! Please enter : \n mkdir <dir_name>\n");
			else if(makeDirectory(args[1])==-1)
				printf("ERROR in creating directory !\n");
		}

		else if(strcmp(args[0],"rmdir")==0)
		{
			if(args[1]==NULL)
				printf("Invalid Command ! Please enter : \n rmdir <dir_name>\n");
			else if(removeDirectory(args[1])==-1)
				printf("ERROR in deleting directory !\n");
		}

		else if(strcmp(args[0],"history")==0)
		{
			if(showHistory(args[1])==-1)
				printf("ERROR ! Trouble showing history !\n");
		}
		
		else executeCommand(args);
	}

}


void main()
{
	loop_SHELL();
}