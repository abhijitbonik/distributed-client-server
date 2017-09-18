#include <my_global.h>
#include <mysql.h>
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
using namespace std;

//function to serve each new client -> multiprocess server
void callServer(int); 

//function to make call to key-value server
char* SendToServer(int,int oper,char* key,char * value);

//structure to maintain information of key-value server
struct conf 
{
 string ip_add;
 int port;
 char start;
 char end; 
};

//vector to store list of all key-value servers
vector<struct conf> servers;

//function to parse key-value server from server.conf 
void readconfigurations(string filename)
{
	std::ifstream myfile(filename.c_str());
	string line="";
	if (myfile.is_open())
	{
		while ( getline (myfile,line) )
		{	
			size_t index=-1;
			struct conf temp; 

		    	index=line.find (' ');
			temp.ip_add=line.substr(0,index);
			line=line.substr(index+1);
			
			index=line.find (' ');
			temp.port=atoi(line.substr(0,index).c_str());
			line=line.substr(index+1);

			index=line.find (' ');
			temp.start=(line.substr(0,index))[0];
			line=line.substr(index+1);
			
			temp.end=line[0];
			servers.push_back(temp);
		}
		//cout<<"cf"<< servers.size();
		myfile.close();
  	}
}

void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno, clilen, pid;
    struct sockaddr_in serv_addr, cli_addr;
    readconfigurations("server.conf");
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) 
       error((char *)"ERROR opening socket");

    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

	if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
		error((char *)"ERROR on binding");

    listen(sockfd,100);

    clilen = sizeof(cli_addr);
	//creates new process for each new client
	while (1) 
	{
		newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr,(socklen_t*)&clilen);
		if (newsockfd < 0) 
			printf("ERROR on accept");
    		pid = fork();

		if (pid < 0)
			printf("ERROR on fork");
		else if (pid == 0)  {
			close(sockfd);
	    	callServer(newsockfd);
	      	exit(0);
	    }
		else close(newsockfd);
	} 
    return 0; 
}

//function to called for each new client 
void callServer (int sock)
{
	int n;
	char key[256];
	int choice=0;
	char buffer[256];
	do
	{
		bzero(buffer,256);
		n = read(sock,buffer,255);
		if (n < 0) cout <<"ERROR reading from socket";

		if(buffer[0]!='9')
			choice =buffer[0]-48;
		else 
			choice =buffer[1]-48+90;

		stringstream test(buffer);
		string segment;
		vector<string> result;
		while(getline(test, segment, '@'))
		{
			result.push_back(segment);
		}
		char * first= (char *)result[1].c_str();
		char * second= (char *)result[2].c_str();

		fflush(stdout);
		cout <<"choice - >" << choice<<" "<<result[1] <<" "<<result[2] << endl;
		cout<<"\nswitch start" << endl;
		fflush(stdout);
		switch(choice)
		{
			case 1:
			{
				MYSQL *con = mysql_init(NULL);
				if (con == NULL)
					n = write(sock,"Connection establishment Failed",255);
				else 
				{
					if (mysql_real_connect(con, "localhost", "root", "",NULL, 0, NULL, 0) == NULL)
						write(sock,"Error connecting database",255);
					else if (mysql_query(con, "Use Key_Value_Logins")) 
						n = write(sock,"Error with database",255);
					else
					{
			  			char query[255];
			  			strcpy(query,"insert into Login_Details(username,password) values('");
			  			strcat(query,first);
			  			strcat(query,"','");
			  			strcat(query,second);
			  			strcat(query,"')");
			  			if (mysql_query(con, query)) 
			  			    n = write(sock,"Error while inserting",255);
			  			else
			  				n = write(sock,"Successfully Inserted",255);
					}
					mysql_close(con);
				}
			}
			break;
			case 2:
			{
				MYSQL *con = mysql_init(NULL);
				if (con == NULL)
					n = write(sock,"Connection establishment Failed",255);
				else
				{
					if (mysql_real_connect(con, "localhost", "root", "",NULL, 0, NULL, 0) == NULL)
						write(sock,"Error connecting database",255);
					else if (mysql_query(con, "Use Key_Value_Logins")) 
						n = write(sock,"Error with database",255);
					else
					{
		  				char query[255];
		  				strcpy(query,"select * from Login_Details where username='");
		  				strcat(query,first);
		  				strcat(query,"' and password ='");
		  				strcat(query,second);
		  				strcat(query,"'");
						cout << query;
		  				if (mysql_query(con, query)) 
			  			    n = write(sock,"Error while fetching",255);
						else
						{
							MYSQL_RES *confres = mysql_store_result(con);
							int totalrows = mysql_num_rows(confres);
							if(totalrows>0)
								n = write(sock,"Login Success",255);
							else 
								n = write(sock,"Login Failed",255);
						}
					}
					mysql_close(con);
				}
			}
			break;
			case 91:
				SendToServer(sock,91,first,second);
				break;
			case 92:
				SendToServer(sock,92,first,second);
				break;
			case -48:
				return;
				break;	
			case 3:
				return;
			case 93:
				return;
			case 94:
				SendToServer(sock,94,first,second);
				break;
			case 95:
				SendToServer(sock,95,first,second);
				break;
			default:
				printf("Please enter correct choice.");
		};
	
	cout<<"\nswitch end";
	fflush(stdout);
    } while (choice!=3 || choice!=93 || choice !=-48);

}

char* SendToServer(int sock,int oper,char* key,char * value)
{
	fflush(stdout);
	char start=tolower(key[0]);
	struct conf temp;
	for(int i=0;i<servers.size();i++)
	{
		temp=servers[i];
		if(temp.start<=start && temp.end>=start)
			break;
	}
	cout << temp.ip_add<<" " <<temp.port<<" "<<temp.start<<" "<<temp.end;
	fflush(stdout);
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char buffer[256];
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		printf((char*)"ERROR opening socket");
	server = gethostbyname(temp.ip_add.c_str());
	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host\n");
       		exit(0);
    	}
    	bzero((char *) &serv_addr, sizeof(serv_addr));
    	serv_addr.sin_family = AF_INET;
    	bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
    	serv_addr.sin_port = htons(temp.port);
    	if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
        	printf((char*)"ERROR connecting");	
	fflush(stdout);
	switch (oper)
	{
		case 91:
			cout<<"inside 91";
			fflush(stdout);
			bzero(buffer,256);
			strcat(buffer,"1@");	
			strcat(buffer,key);
			strcat(buffer,"@");
			strcat(buffer,value);
			n = write(sockfd,buffer,strlen(buffer));

			cout<< buffer <<endl;
			fflush(stdout);

			if (n < 0) 
	         		printf("ERROR writing to socket");
			cout <<"send to database " << n << " "<<sockfd<<endl;
			fflush(stdout);
			bzero(buffer,256);
			n = read(sockfd,buffer,255);
			if (n < 0) 
		 		printf("ERROR reading from socket");
			cout <<"received from database " << n <<endl;
			fflush(stdout);
			n = write(sock,buffer,strlen(buffer));
			if (n < 0) 
	         		printf("ERROR writing to socket");
			cout <<"send to client " << n<< endl;
			fflush(stdout);
			break;
		case 92:
			cout<<"inside 92";
			fflush(stdout);
			bzero(buffer,256);
			strcat(buffer,"2@");	
			strcat(buffer,key);
			strcat(buffer,"@");
			strcat(buffer,value);
			n = write(sockfd,buffer,strlen(buffer));
			bzero(buffer,256);
			n = read(sockfd,buffer,255);
			cout<<buffer;
			fflush(stdout);
			if (n < 0) 
		 		printf((char*)"ERROR reading from socket");
			n = write(sock,buffer,strlen(buffer));
			
			break;
		case 94:
			cout<<"inside 94";
			fflush(stdout);
			bzero(buffer,256);
			strcat(buffer,"4@");	
			strcat(buffer,key);
			strcat(buffer,"@");
			strcat(buffer,value);
			cout  << "before "<< buffer;
			fflush(stdout	);
			n = write(sockfd,buffer,strlen(buffer));

			bzero(buffer,256);
			n = read(sockfd,buffer,255);
			cout<<"read "<<buffer;
			fflush(stdout);
			if (n < 0) 
		 		printf((char*)"ERROR reading from socket");
			n = write(sock,buffer,strlen(buffer));
			break;
		case 95:
			cout<<"inside 95";
			fflush(stdout);
			bzero(buffer,256);
			strcat(buffer,"5@");	
			strcat(buffer,key);
			strcat(buffer,"@");
			strcat(buffer,value);
			n = write(sockfd,buffer,strlen(buffer));

			bzero(buffer,256);
			n = read(sockfd,buffer,255);
			cout<<buffer;
			fflush(stdout);
			if (n < 0) 
		 		printf((char*)"ERROR reading from socket");
			n = write(sock,buffer,strlen(buffer));
			break;
	}
	return NULL;
}
