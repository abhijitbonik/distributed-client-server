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

//mutex variable
//pthread_mutex_t mutexread  = PTHREAD_MUTEX_INITIALIZER;// for reading
//pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; //for writting


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;                
pthread_mutex_t db = PTHREAD_MUTEX_INITIALIZER;                   
int count=0;

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
struct param {
    int arg1;
};

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
	cout<<"Waiting to enter " << pthread_self()<<endl;
	fflush(stdout);

	pthread_mutex_lock( &db );
	cout<<"Inside  " << pthread_self()<<endl;
	fflush(stdout);
	
	hash[key]=value;
	serialize(hash,filename);
	cout<<"Exiting  " << pthread_self()<<endl;
	fflush(stdout);

	sleep(10);
	cout<<"Sleeping "<< pthread_self()<<endl;
	pthread_mutex_unlock(&db);

	cout<<"Exited  " << pthread_self()<<endl;
	fflush(stdout);
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
	
	cout<<"Waiting to delete " << pthread_self()<<endl;
	fflush(stdout);

	pthread_mutex_lock( &db );
	
	it = hash.find (key);   
	if(it==hash.end())  
		val= "Key not found";
	else 
	{
		cout<<"Inside delete " << pthread_self()<<endl; 
		fflush(stdout); 
		hash.erase (it);
		serialize(hash,filename);
	
		cout<<"Exiting from delete " << pthread_self()<<endl;
		fflush(stdout);
		val="Successfully deleted";
	}
	sleep(10);
	cout<<"Sleeping "<< pthread_self()<<endl;
	pthread_mutex_unlock( &db);
	
	
     	
	
  	cout<<"Exited from delete " << pthread_self()<<endl;
	fflush(stdout);
	
	return val;
}

string search(map<string,string> hash,string key)
{
	string s;	
	cout<<"waiting to search "<< pthread_self()<<endl;
	fflush(stdout);
	map<string,string>::iterator it;

	pthread_mutex_lock( &mutex );
     	count++;
	if(count==1)
		     pthread_mutex_lock(&db);
	pthread_mutex_unlock(&mutex);

	cout<<"Insidesearch"<< pthread_self()<<endl;
	fflush(stdout);
	it = hash.find (key); 

	if(it != hash.end())     
   		s= it->second;
	else 
		s= "Key not found";
	
	cout<<"Exiting from search "<< pthread_self()<<endl;
	fflush(stdout);
	
	sleep(10);
	cout<<"Sleeping "<< pthread_self()<<endl;

	pthread_mutex_lock(&mutex);
	count--;
	
	if(count==0)
		pthread_mutex_unlock(&db);
	pthread_mutex_unlock(&mutex);
	
	cout<<"Exited from search "<< pthread_self()<<endl;
	fflush(stdout);
	return s;
}

int main(int argc, char *argv[])
{
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
		newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr,(socklen_t*)&clilen);
		if (newsockfd < 0) 
			error((char *)"ERROR on accept");
		else 
		{
			//creating thread for new requests
			pthread_t some_thread;
    			struct param arg;
			arg.arg1=newsockfd;

    			if (pthread_create(&some_thread, NULL, &callServer, (void *)&arg) != 0) 
        			printf("Error in thread creation\n");
		}
	} 
    return 0; 
}

// function to be executed by thread
void* callServer (void *args)
{
	struct param *arg =(struct param *)args;
	int sock=arg->arg1;
	int n,choice=0;
	char buffer[256];
	bzero(buffer,256);
	n = read(sock,buffer,255);
	
	if (n < 0) error((char *)"ERROR reading from socket");

	choice =buffer[0];
	choice =choice -48;
	//printf("Choice ->%d \n",choice);
	//fflush(stdout);
	

	string str(buffer);
	stringstream test(buffer);
	string segment;
	vector<string> result;
	while(getline(test, segment, '@'))
	{
		result.push_back(segment);
	}
	
	switch(choice)
	{
		case 1: //search
		{
			stringstream ss;
			ss << search(hash,result[1]);
			string s = ss.str();
			//cout<<s;
			//fflush(stdout);
			write(sock,s.c_str(),255);
			if (n < 0) error((char*)"ERROR writing to socket");
				break;
		}
		case 2: //insert
		{
			string value=(result[2].c_str());
			stringstream ss;
		
			//for (std::map<string,string>::iterator it=hash.begin(); it!=hash.end(); ++it)
    			//cout << it->first << " " << it->second<<endl;
	

			ss << insert(hash,"database.db",result[1],value);
			string s = ss.str();
	
			//for (std::map<string,string>::iterator it=hash.begin(); it!=hash.end(); ++it)
    			//cout << it->first << " " << it->second<<endl;
			//fflush(stdout);
			
			write(sock,s.c_str(),255);
   			if (n < 0) error((char*)"ERROR writing to socket");
				break;
		}
		case 4: //delete
		{
			//for (std::map<string,string>::iterator it=hash.begin(); it!=hash.end(); ++it)
    			//cout << it->first << " " << it->second<<endl;
			
			stringstream ss;
			ss << del(hash,"database.db",result[1]);
			string s = ss.str();
			
			//for (std::map<string,string>::iterator it=hash.begin(); it!=hash.end(); ++it)
			//cout << it->first << " " << it->second<<endl;
			//fflush(stdout);
			
			write(sock,s.c_str(),255);
			if (n < 0) error((char*)"ERROR writing to socket");
				break;
		}
		case 5: //update
		{
				
			string value=(result[2].c_str());
			stringstream ss;
	
			//for (std::map<string,string>::iterator it=hash.begin(); it!=hash.end(); ++it)
    			//cout << it->first << " " << it->second<<endl;
			//fflush(stdout);
				
			ss << update(hash,"database.db",result[1],value);
			//for (std::map<string,string>::iterator it=hash.begin(); it!=hash.end(); ++it)
    			//cout << it->first << " " << it->second<<endl;
				string s = ss.str();
			//for (std::map<string,string>::iterator it=hash.begin(); it!=hash.end(); ++it)
    			//cout << it->first << " " << it->second<<endl;
			//fflush(stdout);
			
			write(sock,s.c_str(),255);
			if (n < 0) error((char*)"ERROR writing to socket");
				break;		
		}
		default:
			printf("Please enter correct choice.");
	}
	return 0;
}

