#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <time.h>

#define UP 1
#define DOWN 2

struct clients{
	pid_t pid;
	char chat_id[10];
};

struct message{
	long mtype;
	char mtext[50];
};


int main(){
	int msgid_up,msgid_down,count=0;
	struct clients client_list[50];
	struct msqid_ds qstat;
	char user_list[50] = "LIST ";
	msgid_up=msgget((key_t)UP,IPC_CREAT|0666);
	msgid_down=msgget((key_t)DOWN,IPC_CREAT|0666);
	//printf("q=%d\n",msgid_up);
	//printf("q=%d\n",msgid_down);

	while(1){
		int i=0;
		char *command;
		struct message buff;
		msgrcv(msgid_up,&buff,100,0,0);
		//printf("%s\n",buff.mtext);
		if(buff.mtype==1){
			command=strtok(buff.mtext," ");
			command=strtok(NULL,"\n");
			//client_list[count].chat_id=command;
			strcpy(client_list[count].chat_id,command);
			msgctl(msgid_up,IPC_STAT,&qstat);
			client_list[count].pid=qstat.msg_lspid;
			//printf("%s\n",client_list[count].chat_id);
			printf("New client's Chat ID:- %s\n",command);
			printf("New client's process ID:- %d\n",qstat.msg_lspid);
			printf("Total number of clients:- %d\n",count+1);
			strcat(user_list,client_list[count].chat_id);
			strcat(user_list," ");
			printf("user ID:- %s\n",user_list );
			count++;
			for(i=0;i<count;++i){
				struct message new_msg;
				new_msg.mtype = client_list[i].pid;
				strcpy(new_msg.mtext,user_list);
				msgsnd(msgid_down,&new_msg,50,0);
			}
		}
		else{
			int process_id;
			command=strtok(buff.mtext," ");
			command=strtok(NULL," ");
			//printf("%s\n", command);
			char message[100]="";
			char receiver[15];

			char **params = (char**)malloc(sizeof(char*));
		    i = 0;
		    //int len_message = strlen(command);
		    //int len_receiver = 0;
			while(command!=NULL){
				params[i] = command;
				strcat(message,params[i]);
				strcat(message," ");
				command = strtok(NULL," ");
				i++;
			}
			params[i-1]=strtok(params[i-1],"\n");
			strcpy(receiver,params[i-1]);
			//message[i-1] = NULL;
			printf("%s\n", message);
			printf("%s\n", receiver);
			for ( i = 0; i < count; ++i)
			{
				if(strcmp(client_list[i].chat_id,receiver)==0){
					process_id = client_list[i].pid;
					struct msqid_ds new_qstat;
					
					msgctl(msgid_up,IPC_STAT,&new_qstat);
					char send_time[15]; 
					struct tm tm;
      				tm = *localtime(&new_qstat.msg_stime);
					sprintf(send_time,"%d",tm);
					strcat(message,send_time);
					printf("%s\n",message );
					struct message new_msg1;
					new_msg1.mtype = process_id;
					strcpy(new_msg1.mtext,message);
					printf("%ld %s",new_msg1.mtype,new_msg1.mtext);
					msgsnd(msgid_down,&new_msg1,50,0);
					printf("%ld %s",new_msg1.mtype,new_msg1.mtext);

				}
			}
		}
		//printf("hii\n");
	}

	return 0;

}