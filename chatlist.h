#ifndef CHATINFO_H
#define CHATINFO_H

#include <event.h>
#include <list>

#include"chat_database.h"

using namespace std;

struct User
{
	string name;
	struct bufferevent *bev;//
};
typedef User user;

struct GroupUser
{
	string name;
};
typedef GroupUser groupuser;

struct Group
{
	string name;
	list<groupuser>* l;
};
typedef Group group; 

class ChatInfo
{

public:
	list<user> * online_user;//在线用户
	list<group> * group_info;//群信息

	ChatDataBase *mydatabase;//数据库
public:
	ChatInfo();
	~ChatInfo();
};

#endif