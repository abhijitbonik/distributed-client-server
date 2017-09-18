CREATE DATABASE Key_Value_Logins;
Use Key_Value_Logins;
Create table Login_Details (username varchar(100) Primary Key ,password varchar(100));
Insert into Login_Details(username,password) values('admin','admin');
Insert into Login_Details(username,password) values('user','user');
