#include <stdio.h>
#include <sys/ipc.h> 
#include <sys/msg.h>
#include <string.h>
#include <unistd.h>
#define UP 1
#define DOWN 2

struct message{
	long mtype;
	char mtext[50];
};

int main(){
	char client_name[15];
	int p,qid_up,qid_down;
	struct message msg;

	qid_up=msgget((key_t)UP,IPC_CREAT|0666);
	printf("%d\n",qid_up );

	qid_down=msgget((key_t)DOWN,IPC_CREAT|0666);
	printf("%d\n",qid_down );

	msg.mtype = 1;
	strcpy(msg.mtext,"NEW ");
	printf("Enter new user Chat ID :- ");
	scanf("%s",client_name);
	strcat(msg.mtext,client_name);
	//printf("%d\n",getpid() );
	int msgsize  = sizeof(msg);
	//sprintf("%d\n",msgsize);
	//printf("%s\n",msg.mtext);
	p = msgsnd(qid_up,&msg, msgsize, 0);
	struct message online_users;
	msgrcv(qid_down,&online_users,50,0,0);

	while(1){
		printf("List of clients :- %s\n",online_users.mtext);
		printf("Pick a client for chat :- ");
		char user[15],message[50];
		scanf("%s",user);
		printf("Plaese type a message for above user :- ");
		scanf("%s",message);

		struct message send_info;
		send_info.mtype=getpid();
		strcpy(send_info.mtext,"MSG ");
		strcat(send_info.mtext,message);
		strcat(send_info.mtext," ");
		strcat(send_info.mtext,user);
		printf("%s\n",send_info.mtext);
		msgsnd(qid_up,&send_info,sizeof(send_info),0);
		sleep(1);

		struct message rcv_msg;
		msgrcv(qid_down,&rcv_msg,50,getpid(),0);
		//printf("%s hem \n",rcv_msg.mtext);

	}



	return 0;
}