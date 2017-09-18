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
#include <map>
#include <sstream>
#include<semaphore.h>
using namespace std;
struct param {
    int arg1;
    char arr[256];
};
sem_t mutex;                
sem_t db;                   
int count=0;
pthread_mutex_t lock;

//function to serve incoming request
void* callServer (void* sock);

//map - a inmemory data-structure to serve as key-value store 
std::map<string,string> hash;

void error(char *msg)
{
    perror(msg);
    exit(1);
}

//structure for passing arguments to thread


// function to write data to file for persistent storage
void serialize(map<string,string> hash,string filename)
{
	ofstream file(filename.c_str());
	for (std::map<string,string>::iterator it=hash.begin(); it!=hash.end(); ++it)
    	file << it->first << " " << it->second<<endl;
  	file.close();
}

// function to read data from file to the map
void deserialize(map<string,string>& hash,string filename)
{
	std::ifstream myfile(filename.c_str());
	string line="";
	if (myfile.is_open())
	{
		while ( getline (myfile,line) )
		{	
			size_t index=-1;
			index=line.find (' ');
			string key=line.substr(0,index);
			line=line.substr(index+1);
			cout << key<<" " << line<<" ";
			hash[key]=(line.c_str());
		}
		myfile.close();
		cout<<endl;
	}  	
}

string insert(map<string,string> &hash,string filename,string key, string value)
{
	sem_wait( &db );
	hash[key]=value;
	serialize(hash,filename);
	sem_post(&db);
	return "Successfully inserted";
}

string update(map<string,string> &hash,string filename,string key, string value)
{
	insert(hash,filename,key,value);
	return "Successfully updated";
}

string del(map<string,string> &hash,string filename,string key)
{	
	string val;
	map<string,string>::iterator it;
	sem_wait( &db );
	it = hash.find (key);   
	if(it==hash.end())  
		val= "Key not found";
	else 
	{
		hash.erase (it);
		serialize(hash,filename);
		val="Successfully deleted";
	}
	sem_post( &db);
  	cout<<"Exited from delete " << pthread_self()<<endl;
	fflush(stdout);
	return val;
}

string search(map<string,string> hash,string key)
{
	string s;	
	map<string,string>::iterator it;
	sem_wait( &mutex );
     	count++;
	if(count==1)
	     sem_wait(&db);
	sem_post(&mutex);
	it = hash.find (key); 
	if(it != hash.end())     
   		s= it->second;
	else 
		s= "Key not found";
	sem_wait(&mutex);
	count--;
	if(count==0)
		sem_post(&db);
	sem_post(&mutex);
	return s;
}

int main(int argc, char *argv[])
{
	sem_init(&mutex, 0, 10000);
	sem_init(&db, 0, 10000);
	pthread_mutex_init(&lock, NULL);
	deserialize(hash,"database.db");
	int sockfd, newsockfd, portno, clilen, pid;
	struct sockaddr_in serv_addr, cli_addr;
	if (argc < 2) 
	{
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

	while (1) 
	{
		pthread_mutex_lock(&lock);
		int newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr,(socklen_t*)&clilen);
		int array[1];
		array[0]=newsockfd;
		pthread_t some_thread;
		if (newsockfd < 0) 
			printf("ERROR on accept");		
		else 
		{
			if (pthread_create(&some_thread, NULL, &callServer, (void *)array) != 0) 
				printf("Error in thread creation\n");
		}
	} 
    return 0; 
}

// function to be executed by thread
void* callServer (void *args)
{	

	int* val=(int *)args;
	int sock=val[0];
	pthread_mutex_unlock(&lock);
	char buffer[256];
	bzero(buffer,256);
	int n = read(sock,buffer,255);
	int choice=0;
	stringstream ss;
	choice =buffer[0]-48;
	
	stringstream test(buffer);
	string segment;
	vector<string> result;
	while(getline(test, segment, '@'))
	{
		result.push_back(segment);
	}
	string value=result[2];
	cout<<"Choice -> "<<choice<<result[1]<<result[2]<< endl;
	switch(choice)
	{
		case 1: //search
			ss << search(hash,result[1]);
			break;
		
		case 2: //insert
			ss << insert(hash,"database.db",result[1],value);
			break;
		case 4: //delete
			ss << del(hash,"database.db",result[1]);
			break;
		case 5: //update
			ss << update(hash,"database.db",result[1],value);
			break;
		default:
			printf("Please enter correct choice.");
	}
	string s = ss.str();
	cout<<s<< pthread_self()<<endl;
	n=write(sock,s.c_str(),255);
	if (n < 0) cout <<"ERROR writing to socket";
	close(sock);
}

