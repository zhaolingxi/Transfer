#ifndef SERVER_H
#define SERVER_H

#include <event.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <json/json.h>
#include <jsoncpp/json/json.h>

#include<event2/listener.h>

#include<thread>
#include<unistd.h>
#include<iostream>

#include"chatlist.h"

using namespace std;
#define IP "172.17.239.60"
#define PORT 8000
#define MAXSIZE 1024 * 1024

class Server
{
private:
	struct event_base *base;//事件集合
	struct evconnlistener *listener;//监听事件
	
	static ChatInfo* chatlist;//链表对象
	static ChatDataBase* chatdb;//数据库对象

	/*
	* 监听接入的tcp链接的回调函数，当tcp链接进入时，触发函数
	* @param listener：监听者
	* @param fd：当前socket的fd
	*/
	static void listener_cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr * addr, int socklen, void * arg);

	/*
	* 处理客户端的函数
	* @param fd：该客户端对应的fd
	*/
	static void client_handler(int fd);

	/*
	* 处理客户端文件传输的函数
	* @param fd：该客户端对应的fd
	*/
	static void send_file_handler(int, int, int* from_fd, int* to_fd);


	/*
	* 回复客户端
	*/
	static void reply_client(struct bufferevent* bev,string replystr);


	static void read_cb(struct bufferevent *bev, void *ctx);
	static void event_cb(struct bufferevent *bev, short what, void *ctx);

	/*
	* 服务器注册
	*/
	static void server_register(struct bufferevent *bev,Json::Value val);

	/*
	* 服务器登录
	*/
	static void server_login(struct bufferevent *bev,Json::Value val);

	/*
	* 服务器添加好友
	*/
	static void server_add_friend(struct bufferevent* bev, Json::Value val);

	/*
	* 服务器创建群聊
	*/
	static void server_create_group(struct bufferevent* bev, Json::Value val);

	/*
	* 服务器添加群聊
	*/
	static void server_add_group(struct bufferevent* bev, Json::Value val);

	/*
	* 服务器私聊
	*/
	static void server_private_chat(struct bufferevent* bev, Json::Value val);

	/*
	* 服务器群聊
	*/
	static void server_group_chat(struct bufferevent* bev, Json::Value val);


	/*
	* 服务器获取群成员
	*/
	static void server_get_group_member(struct bufferevent* bev, Json::Value val);

	/*
	* 服务器处理客户端下线
	*/
	static void server_offline(struct bufferevent* bev, Json::Value val);

	/*
	* 服务器处理发送文件
	*/
	static void server_send_file(struct bufferevent* bev, Json::Value val);
public:
	Server(const char*ip="127.0.0.1",int port=8000);
	~Server();
};

#endif
