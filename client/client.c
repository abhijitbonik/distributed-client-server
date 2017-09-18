#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
void error(char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
	int sockfd, portno, n;
	char c;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	int isLogin=0;
	char buffer[256];
	if (argc < 3) {
		fprintf(stderr,"usage %s hostname port\n", argv[0]);
		exit(0);
    	}
	portno = atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		error("ERROR opening socket");
	server = gethostbyname(argv[1]);
	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host\n");
        	exit(0);
    	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
	serv_addr.sin_port = htons(portno);
	if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
	        error("ERROR connecting");
	int choice=0;
	
	do
	{
		bzero(buffer,256);
		char user[256],pass[256];	
		system("clear");
		printf("Enter Your Choice ->\n1.Register\n2.Login\n3.Exit\n\n");
		fflush(stdout);
		scanf("%d",&choice);
		buffer[0]=(char)(choice+48);
		switch(choice)
		{			
			case 1:
				printf("Please enter username-> ");
				scanf("%s",user);
				
				printf("Please enter password-> ");
				scanf("%s",pass);

				strcat(buffer,"@");
				strcat(buffer,user);
	
				strcat(buffer,"@");
				strcat(buffer,pass);

				n = write(sockfd,buffer,strlen(buffer));
				if (n < 0) 
	         		error("ERROR writing to socket");
				
				bzero(buffer,256);
				n = read(sockfd,buffer,255);
				if (n < 0) 
		 			error("ERROR reading from socket");
    				printf("%s\n",buffer);
				
				sleep(10);
					
				break;

			case 2:
				printf("Please enter username-> ");
				scanf("%s",user);

				printf("Please enter password-> ");
				scanf("%s",pass);
				
				strcat(buffer,"@");
				strcat(buffer,user);
	
				strcat(buffer,"@");
				strcat(buffer,pass);

				n = write(sockfd,buffer,strlen(buffer));
				if (n < 0) 
	         			error("ERROR writing to socket");
				
				bzero(buffer,256);
				n = read(sockfd,buffer,255);
	    			if (n < 0) 
	         			error("ERROR reading from socket");
				printf("%s\n",buffer);
				if (strcmp(buffer,"Login Success")==0)
					isLogin=1;
				sleep(10);
					
				break;

			case 3:
				strcat(buffer,"@");
				strcat(buffer," ");
	
				strcat(buffer,"@");
				strcat(buffer," ");

				n = write(sockfd,buffer,strlen(buffer));
				if (n < 0) 
		        	error("ERROR writing to socket");
				printf("Bye\n");
				return 0;
				break;

			default :
				printf("Please enter correct choice.\n\n");
				sleep(2);
				system("clear");
				break;
			
		}

	} while(isLogin==0);

	do
	{
		char key[256],value[256];
		bzero(buffer,256);
		system("clear");
    		printf("Enter Your Choice ->\n1.Search by Key\n2.Insert Key-Value pair\n3.Exit\n4.Delete\n5.Update\n\n");
		scanf("%d",&choice);
		buffer[0]='9';
		buffer[1]=(char)(choice+48);
		switch(choice)
		{
			case 2:
			case 5:
				printf("Enter Key-> ");
				scanf("%s",key);

				printf("Enter value-> ");
				scanf("%s",value);

				strcat(buffer,"@");
				strcat(buffer,key);
	
				strcat(buffer,"@");
				strcat(buffer,value);

				n = write(sockfd,buffer,strlen(buffer));
	    			if (n < 0) 
	         			error("ERROR writing to socket");
				
				bzero(buffer,256);
				n = read(sockfd,buffer,255);
	    			if (n < 0) 
	         			error("ERROR reading from socket");
				printf("%s",buffer);
				fflush(stdout);
				sleep(3);
					
				break;

			case 3:
				strcat(buffer,"@");
				strcat(buffer," ");
	
				strcat(buffer,"@");
				strcat(buffer," ");

				n = write(sockfd,buffer,strlen(buffer));
				if (n < 0) 
		        	error("ERROR writing to socket");
				printf("Bye\n");
				return 0;
				break;
			case 1:			
			case 4:
				printf("Enter key-> ");
				scanf("%s",key);

				strcat(buffer,"@");
				strcat(buffer,key);
	
				strcat(buffer,"@");
				strcat(buffer,key);

				n = write(sockfd,buffer,strlen(buffer));
				if (n < 0) 
	         			printf("ERROR writing to socket");

				bzero(buffer,256);
				n = read(sockfd,buffer,255);
 				if (n < 0) 
	         			error("ERROR reading from socket");

				printf("%s",buffer);
				fflush(stdout);
				sleep(3);
					
				break;
			default:
				printf("Please enter correct choice.");
		
		}
	} while(choice!=3);
    return 0;
}
