//omri mizrahi
//303082549
//ex4

#include <signal.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <netdb.h>
#include <memory.h>
#include <stdio.h> //printf
#include <string.h> //memset
#include <stdlib.h> //exit(0);
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ctype.h> 
#include <sys/select.h>
#define BUFLEN 4096  //Max length of buffer

//struct node to hold the data client and the client itself anf the pointer to the next node 
typedef struct Node_t{
    char* data;
    struct sockaddr *client;
    struct Node_t *prev;
}NODE;
//struct queue that hold the nodes and pointers to the first and last node,and queue size 
typedef struct queue {
    NODE *head;
    NODE *tail;
    int size;
}Queue;
 
 
 Queue* queue;
 //------------------------------------------------------------------------------------------------//
void die(char *s)
{
    perror(s);
    exit(1);
}

//------------------------------------------------------------------------------------------------//
//turn string to upper case letters
void toUpperCase(char* str)
{
	int j;
	if(str==NULL)
		die("String error\n");
	for(j=0;j<strlen(str)||str[j]!='\0';j++)
	{
		str[j]=toupper(str[j]);
	
	}

	
}

//------------------------------------------------------------------------------------------------//
//create new node and add it to the end of the queue(if queue is empty the firtt is the last node)  
void addToQueue(Queue* q,char *msg, struct sockaddr *client)
{
	if(q==NULL||msg==NULL)
		die("error adding to queue\n");
	NODE * new_n=(NODE*)malloc(sizeof(NODE));
	if(new_n==NULL)
    		die("malloc error");
	toUpperCase(msg);
	new_n->data=strdup(msg);
	new_n->prev=NULL;
	new_n->client=client;
	if(q->size==0)
	{
		q->head=new_n;
		q->tail=new_n;
		q->size=1;
		return ;
	}
	q->tail->prev=new_n;
	q->tail=new_n;
	q->size++;
	return ;
}
//------------------------------------------------------------------------------------------------//

//check if queue is empty return 1 if it is 0 elsewere
int isEmpty(Queue* q)
{
	 if((q->head==NULL)&&(q->tail==NULL)&&(q->size==0))
	 	return 1;
	 return 0;
}
//------------------------------------------------------------------------------------------------//
//remove the first node in queue 
void removeFromeQueue()
{


	NODE* temp_q;
	temp_q=queue->head;
	/*if(isEmpty(queue))
		die("queue is empty\n");*/
	//free(temp_q->data);
	if(queue->size==1)
	{
		
		//free(queue->head);
		queue->head=NULL;
		queue->tail=NULL;
		queue->size=0;
		free(temp_q->data);
		free(temp_q);
		
		return ;
	
	}else
	{

		//queue->head=temp_q;
		queue->head=queue->head->prev;
		free(temp_q->data);
		free(temp_q);
		queue->size--;
		return ;
	}
}
//------------------------------------------------------------------------------------------------//
/*int firstInQueue(Queue* q,int port)
{
	if(q->head->port==port)
		return 1;
	return 0;
}*/
//------------------------------------------------------------------------------------------------//
//global variable of queue


//free queue whan got signal handler 
void freeQueue()
{
	NODE* tN;
	while(queue->size>0)
	{
		tN=queue->head;
		queue->head=queue->head->prev;
		free(tN->data);
		free(tN);
		
		queue->size--;	
	}
	//if(queue->size)
	free(queue);

}
//exit with signal handler
void sig_init(int sig)
{ 
	freeQueue();
	//free(queue);
	exit(0);
}
int main(int argc, char* argv[])
{
    
    
    if(argv[1]==NULL||argc !=2)//cmd " 'input_file' 'host name' 'num_of_tries' "
		die("Wrong Input");
    int port=atoi(argv[1]);
    //struct sockaddr_in si_me, si_other;
     struct sockaddr_in client, server;
     socklen_t clientLength;
    int fd,  slen = sizeof(server) ;
    fd_set readfds,writefds;
    //create a UDP socket
    if ((fd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)	//create socket descriptor
    {
        die("socket");
    }
     
    // zero out the structure
    memset((char *) &server, 0, sizeof(server));
    
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr =INADDR_ANY;
     
    //bind socket to port
    if( bind(fd , (struct sockaddr*)&server, slen ) == -1)
    {
        die("bind");
    }
    clientLength=sizeof(struct sockaddr_in);
    queue=(Queue*)malloc(sizeof(Queue));
    if(queue==NULL)
    	die("malloc error");
   // queue=qu;
    queue->head=NULL;
    queue->tail=NULL;
    queue->size=0;
    int recieve,numfd = fd + 1,bytes_recieved;
    //keep listening for data
    char msg[BUFLEN];
    char recieve_data[BUFLEN];
    printf("server is ready\n\n ");
	signal(SIGINT,sig_init);  //signal handler ^c 
    while(1)
    {

	
	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	FD_SET(fd, &readfds);
     	if(!(isEmpty(queue)))
     	{
     		FD_SET(fd, &writefds);
     	}
     
     	recieve = select(numfd, &readfds,&writefds, 0, 0);//select block until tere is somthing ro read or write
    	if (recieve == -1) 
   	 {
   	   perror("select error"); // error occurred in select()
   	 } 
	
     				 // one or both of the descriptors have data
     	if (FD_ISSET(fd, &readfds)) //if set to read
      	{		
       				 // FD_CLR(fd, &readfds); //clear the set
      				   printf("server is ready to read...\n ");	//recieve from client
      				  bytes_recieved = recvfrom(fd,recieve_data, sizeof(recieve_data),0,(struct sockaddr *)&client,&clientLength);
      				  if(bytes_recieved<0)
					 die("reciver falied");

      				  recieve_data[bytes_recieved]= '\0';
				 // printf("%s\n",recieve_data);
				  addToQueue(queue,recieve_data,(struct sockaddr *)&client); //add message to queue according spacipic client to the last node in the queue
				  memset(msg, '\0', sizeof(msg));
				  strcpy(msg, recieve_data);
				   //printf("%s\n",queue->head->data);
				  
     	}

        if (FD_ISSET(fd, &writefds)==1 && isEmpty(queue)==0 ) //if set to write
     	{
     				  
     				// FD_CLR(0, &writefds);
   				 printf("server is ready to write...\n ");
   				// printf("%s",queue->head->data);
        			 if(sendto(fd, msg, sizeof(msg), 0, queue->head->client, clientLength)<0)	//send to spacipic client the head of the queue 		  
					 die("sender falied");
				removeFromeQueue();	//remove first node in queue

     	}
     	printf("\n\n");

    }
   // free(queue);
    close(fd);
    return 0;
}
