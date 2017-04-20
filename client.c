//omri mizrahi
//303082549
//ex2

#include <limits.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <ctype.h>
#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"


//check if the port ar time is in numbers and not string
int is_number_check(char* s);
  
// This function  gets the  <time-interval> and change it to the correct format in to buf. 
void getTime(char* t, char* buf);

//change the "http://" to lower case letters in the URL return NULL if there is no http://
char* httpCheck(char* url);

// Function that gets the details from URL and set host-name , port number, and page return error if theres wrong input like no http://.
void url_builder(char* url, char** hostName, int* port, char** page);



int main(int argc, char* argv[])
{
	if(argc > 5 || argc < 2)						// To much or too less arguments.
	{
		fprintf(stderr,"Usage: client [-h] [-d <time-interval>] <URL>\n");
		exit(1);
	}
	char tbuf[128];									// this string will save the time Format (in GMT).
	int i;										
	int HttpFlag = 0, HeadFlag = 0, Dflag = 0;
	char *hostname;						
	int port = 80;						//server port (defult is 80)
	char *page;										// Page of the URL.
	char *req = "GET /";									// type of the request .
	int fd;											// Socket Descriptor.
	char buf[256];									//buf for the recived data from server.
	char msg[256*2]="";								//the message that we will send to the server.
	struct sockaddr_in server;
	struct hostent* hp;
	char *modifySince = "\r\nIf-Modified-Since: ";				// whan the last modification(if is nedded) .
	for(i = 1; i < argc; i++)
	{
		if(strcmp(argv[i],"-h") == 0 && HeadFlag == 0)			//case 1: if "-h" Found.
		{
			HeadFlag += 1;
			req = "HEAD /";
		}
		else if(strcmp(argv[i],"-d") == 0 && Dflag == 0)		//case 2: if "-d" Found.
		{
			if(i == argc-1)									// error if "-d" is in the last place in the array(so there is no <time-interval> ).
			{
				fprintf(stderr,"Usage: client [-h] [-d <time-interval>] <URL>\n");
				exit(1);
			}
			Dflag+= 1;
			if(strcmp(argv[i+1],"-h") == 0 || httpCheck(argv[i]) != NULL)		//error if the next field after "-d" is not <time-interval>
			{
				fprintf(stderr,"Usage: client [-h] [-d <time-interval>] <URL>\n");
				exit(1);
			}
			getTime(argv[i+1], tbuf);			
			i++;			//increase index becuse we have the <time-interval> .						
		}
		else if((strstr(argv[i],"www") != NULL || (httpCheck(argv[i])!= NULL)) && HttpFlag == 0)	//case 3: if URL Found.
		{

			url_builder(argv[i], &hostname, &port, &page);
			if((strstr(argv[i],"http://") != argv[i] ))
			{
			
				fprintf(stderr,"Wrong Input\n");
				exit(1);
				
			}	
			HttpFlag+=1;	
		}
		else										// input is incorrect order (Usage ERROR). 
		{
			fprintf(stderr,"Usage: client [-h] [-d <time-interval>] <URL>\n");
			exit(1);
		}
	}	
					
	if((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {		//create socket descriptor 
		perror("create socket error\n");			//error if socket creation faild
		exit(1);
	}
	hp = gethostbyname(hostname);							// get the ip adresses of the hostname.
	if(hp == NULL)
	{
		herror("gethostbyname error\n");
		exit(1);
	}
	if(port<1)
	{
		fprintf(stderr,"Wrong Input\n");
		exit(1);
	}
	server.sin_port = htons((int)port);
	server.sin_family = AF_INET;							// IPv4.
	memcpy(&server.sin_addr, hp->h_addr_list[0], sizeof(server.sin_addr));		// Copy from hp to server the IP adress.
	//printf("%d\n",port);
	if(connect(fd, (struct sockaddr*)&server, sizeof(server)) < 0)			// Connect to the server.
	{										//error if connection faild
		perror("Connection error!\n");
		close(fd);
		exit(1);
	}
	
	strcat(msg,req);
	strcat(msg,page);												// Concat the page to the message.
	strcat(msg," HTTP/1.0\r\nHost: ");										// Concat the all rellevant stuff into the message.
	strcat(msg,hostname);												// Concat the hostName into the message.
	if(Dflag == 1)									// if -d flag has been found.
	{	

		strcat(msg,modifySince);						// Concat the modify type to the message.						
		strcat(msg,tbuf);							// Concat the modify message to the message.
	}
	
	strcat(msg,"\r\n\r\n");	
	//printf("\n");						// Concat the rellevant stuff into the message.
	printf("HTTP request =\n%s\nLEN = %d\n", msg, (int)strlen(msg));	
	if(write(fd, msg, sizeof(msg)) < 0)					// Send the message to the server.					
	{									//error if connection faild
		perror("sending error\n");
		close(fd);
		exit(1);
	}
	int nbytes = 0;
	int size = 0;
	do
	{
		nbytes = recv(fd,buf,sizeof(buf),0);	        			// read recived data fron server into the buf.
		if(nbytes < 0)
		{
			perror("Recive error\n");
			close(fd);
			exit(1);
		}
		size+= nbytes;									// Count the amount of the bytes total recieved.
		printf("%s",buf);
		memset(buf,'\0',strlen(buf));							// Prints the buffer to the screen.
	}while(nbytes > 0);								// While we have more data to read from the server.							
	printf("\n Total received response bytes: %d\n",size);
	close(fd);									// Close the socket descriptor.
	return 0;
}




//check if the port ar time is in numbers and not string
int is_number_check(char* s)
{
	int i=0;
	if(strstr(s,"-")!=NULL)
	{
		i=1;
	}
	for(;i<strlen(s);i++)
	{
		if(!isdigit(s[i]))
		{
			return 0;
		}
	}
	return 1;
}

//change the "http://" to lower case letters in the URL return NULL if there is no http://
char* httpCheck(char* url)
{
	int i=0;
	char *p=url;

	for(;i<4;i++)
	{
		url[i]=tolower(url[i])	;
	}

	return strstr(p,"http://"); 
}


/*------------------------------------------------------------------------------------------------------------------------------------------------------*/
 
// Function that gets the details from URL and set host-name , port number, and page return error if theres wrong input like no http://.
void url_builder(char* url, char** hostName, int* port, char** page)
{

	char *urlHead;
	char *portNum;
	char *p;
	;
	if((urlHead= httpCheck(url)) == NULL)	     // Incorrect URL
	{
		fprintf(stderr,"Wrong Input\n");
		exit(1);
	}
	*hostName = urlHead + strlen("http://");		// Get the Host Name
	p = strchr(*hostName,':');					// Search for ":" in the hostName to find port
	if(p != NULL)								// if found ":" = Port  found.
	{	
		*p = '\0';
		portNum = p+1;
	} 
	else											// ":" not found there is no port.
	{
		p = strchr(*hostName, '/');				// Search for "/" in the hostName to find page.
		if(p != NULL)							// if "/" is found page there is page.
		{
			*p = '\0';
			*page = p+1;
			return;
		}											// The "/" not found there is no page.
		*page = "";
		return ;
	}
	
	p = strchr(portNum,'/');						// The port is found, Search for "/" in the port.
	
	if(p == NULL)								// if Path was not found.
	{
		*page = "";
		if(strcmp(portNum,"") == 0 )					// if port is invalid.
		{
			fprintf(stderr,"Usage: client [-h] [-d <time-interval>] <URL>\n");
			exit(1);
		}
		if(!is_number_check(portNum))
		{
			fprintf(stderr,"Wrong Input\n");
			exit(1);
		}
		*port = atoi(portNum);						// Convert the port from String into number.
		return;
	}
	else											// The portNum is found and the Path was found.
	{
		*p = '\0';
		if(!is_number_check(portNum))
		{
			fprintf(stderr,"Wrong Input\n");
			exit(1);
		}
		if(strcmp(portNum,"") == 0)					
		{
			fprintf(stderr,"Usage: client [-h] [-d <time-interval>] <URL>\n");
			exit(1);
		}

		*port = atoi(portNum);
		*page = p+1;
	}	
}
/*------------------------------------------------------------------------------------------------------------------------------------------------------*/
	
// This function  gets the  <time-interval> and change it to the correct format in to buf. 
void getTime(char* t, char* buf)
{
	time_t now;						
	char timebuf[128];				
	char *day = t, *min, *hour;		
	if((hour= strstr(t,":")) == NULL)		// Search for ":" in the hours.		
	{
		fprintf(stderr,"Wrong  input \n");	// ":" not found 
		exit(1);
	}				// ":" found
	*hour = '\0';	
	hour += 1;
	min = strstr(hour,":");	
	if((min = strstr(hour,":")) == NULL)	// ":" not found 
	{
		fprintf(stderr,"Wrong  input \n");
		exit(1);
	}
	*min = '\0';	// ":" found
	min +=1;
	if(strcmp(day,"") == 0 || strcmp(hour,"") == 0 || strcmp(min,"") == 0 || strstr(min,":") != NULL) // if one or more of the variable is empty String.
	{
		fprintf(stderr,"Wrong  input \n");
		exit(1);
	}
	if(!is_number_check(day) || !is_number_check(hour) || !is_number_check(min))
	{
		fprintf(stderr,"Wrong  input \n");
		exit(1);
	}
	now = time(NULL);
	now = now - (atoi(day) * 24 * 3600 + atoi(hour) * 3600 + atoi(min) * 60);		// Calculate the time.
	strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));		// Convert the time to the right format and save it into timebuf variable.
	strcpy(buf,timebuf);		// Copy from timebuf into buf.
}




/*------------------------------------------------------------------------------------------------------------------------------------------------------*/




