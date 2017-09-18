Install My-Sql Server
	sudo apt-get update
	sudo apt-get install mysql-server
	set root password as root

Install Mysql Libraries for C
	sudo apt-get install libmysqlclient-dev

Create database 	
	Go to the folder containing script.sql
	Login to mysql
	Run following command	
	->source script.sql

Compiling Code:

There are 3 folders client ,database ,server. Client contains client code ,database contains key-value store code , server contains frontend server code

Step 1: run database server with port number
(Assuming you are in code directory)

Currently there are 3 directoy in database directory (all contains same code but different database.db)

	-> cd database/server-1
	-> g++ database.cpp -o database -lpthread
	->./database port_no

Make copy of database directory on as many server as you want as key value store. You can make multiple copies on same server and run it with different port
	
Step 2: Write database server configuration to frontend server

Update the key-value server configuration in server.conf in server directory in specified format 
 	
	ip_address port from_char to_char

	Note :- from_char and to_char are in lowercase.

Step 3: Compiling and running front-end server code

  -> cd ../../server
  -> g++ frontend.cpp -o frontend `mysql_config --cflags --libs`
  -> ./frontend port_no

Step 4: Compiling and running client code

Make as many client as required 

	-> cd ../client
	-> gcc client.c -o client
	-> ./client frontend_server_ip_add frontend_server_port
