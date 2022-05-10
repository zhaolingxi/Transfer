#ifndef CHAT_DATABASE_H
#define CHAT_DATABASE_H

#include<mysql/mysql.h>

#include<iostream>
#include<stdio.h>
#include<string.h>
using namespace std;

#define MAXNUM 1024//最大群个数

class ChatDataBase
{
private:
	MYSQL *mysql;
public:
	ChatDataBase();
	~ChatDataBase();

	void my_database_connect(string name);
	void  my_database_disconnect();

	int my_database_get_group_name(string *);
	void my_database_get_gorup_member(string,string &);

	bool my_database_user_exist(string s);
	void my_database_user_add(string ,string);//添加用户

	bool my_database_is_friend(string, string);
	void my_database_add_new_friend(string, string);

	bool my_database_group_exist(string s);
	void my_database_add_new_group(string, string);

	bool my_database_user_password_correct(string,string);
	string my_database_get_friend(string);

	void my_database_get_friend_group(string, string&, string&);
};

#endif
