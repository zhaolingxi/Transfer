#include "server.h"
#include "json_server.h"

#include <sstream>
ChatDataBase* Server::chatdb = new ChatDataBase();
ChatInfo* Server::chatlist = new ChatInfo();

Server::Server(const char* ip, int port)
{
	//弃用，声明改为静态
	//	chatlist=new ChatInfo();
	//	chatdb=new ChatDataBase();

	//创建事件集合
	base = event_base_new();

	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(ip);

	//创建监听对象
	listener = evconnlistener_new_bind(base, listener_cb, NULL
		, LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, 10, (struct sockaddr*)&server_addr, sizeof(server_addr));

	if (!listener)
	{
		cout << "evconnlistener_new_bind error!" << endl;
	}

	cout << "服务器初始化完成" << endl;
	event_base_dispatch(base);

}

Server::~Server()
{
	//释放服务器端监听
	event_base_free(base);
	cout << "析构完成" << endl;
}

void Server::reply_client(struct bufferevent* bev, string replystr)
{
	cout << replystr.length() << endl;
	if (bufferevent_write(bev, replystr.c_str(), replystr.length()) < 0)
	{
		cout << "error, should reply_client:"<< replystr << endl;
	}
}

void Server::listener_cb(struct evconnlistener* listener, evutil_socket_t fd, struct sockaddr* addr, int socklen, void* arg)
{
	cout << "accpet client connect" << fd << endl;

	//为每一个客户端创建一个线程，用于服务
	thread client_thread(client_handler, fd);

	client_thread.detach();
	//client_thread.join();
}

void Server::client_handler(int fd)
{
	struct event_base* base = event_base_new();
	struct bufferevent* bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);//关闭tcp,释放对象
	if (!bev)
	{
		cout << "bufferevent_socket_new error" << endl;
	}

	bufferevent_setcb(bev, read_cb, NULL, event_cb, NULL);
	bufferevent_enable(bev, EV_READ);
	event_base_dispatch(base);//阻塞
	cout << "send from:"<< fd <<"bye!" << endl;
	event_base_free(base);
}

void Server::send_file_handler(int length, int port, int* from_fd, int* to_fd)
{
	int sockfd =socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd ==-1)
	{
		return;
	}

	int opt = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	// 接收缓冲区
	int nRecvBuf = MAXSIZE;
	setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (const char*)&nRecvBuf, sizeof(int));
	//发送缓冲区
	int nSendBuf = MAXSIZE;    //设置为1M
	setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (const char*)&nSendBuf, sizeof(int));

	struct sockaddr_in server_addr, client_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(IP);


	bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
	listen(sockfd, 10);

	int len = sizeof(client_addr);
	//接受发送客户端的连接请求
	*from_fd = accept(sockfd, (struct sockaddr*)&client_addr, (socklen_t*)&len);
	//接受接收客户端的连接请求
	*to_fd = accept(sockfd, (struct sockaddr*)&client_addr, (socklen_t*)&len);

	char buf[MAXSIZE] = { 0 };
	size_t size, sum = 0;
	while (1)
	{
		size = recv(*from_fd, buf, MAXSIZE, 0);
		if (size <= 0 || size > MAXSIZE)
		{
			break;
		}
		sum += size;
		send(*to_fd, buf, size, 0);
		if (sum >= length)//传输完毕
		{
			break;
		}
		memset(buf, 0, MAXSIZE);
	}

	close(*from_fd);
	close(*to_fd);
	close(sockfd);
}

void Server::GetRandomPort(int& port)
{
	int nStartPort = 8000;
	int nEndPort = 8100;
	/*while (1)
	{
		string psz_port_cmd;
		int i_random_port = rand() % (nEndPort - nStartPort + 1) + nStartPort;
		sprintf(psz_port_cmd, "netstat -an | grep :%d > /dev/null", i_random_port);
		if (!system(psz_port_cmd))
		{
			port = i_random_port;
		}
	}*/

	for (int i = nStartPort; i <= nEndPort; i++)
	{
		char psz_port_cmd[1024];
		sprintf(psz_port_cmd, "netstat -an | grep :%d > /dev/null", i);
		if (!system(psz_port_cmd))
		{
			port = i;
			break;
		}
	}

}

void Server::read_cb(struct bufferevent* bev, void* ctx)
{
	char buf[1024] = { 0 };
	memset(&buf, 0, sizeof(buf));
	int size = bufferevent_read(bev, buf, sizeof(buf));
	if (size < 0)
	{
		cout << "bufferevent_read error" << endl;
	}

	string errs;
	Json::Value val;
	Json::CharReaderBuilder readerBuilder;
	Json::Value def;
	def["emitUTF8"] = true;
	readerBuilder.setDefaults(&def);
	std::unique_ptr<Json::CharReader> jsonReader(readerBuilder.newCharReader());

	if (!jsonReader->parse(buf, buf + (sizeof(buf) / sizeof(char)), &val, &errs))
	{
		cout << "解析错误" << endl;
	}
	string cmd = val["cmd"].asString();
	if (cmd == "register")//注册
	{
		server_register(bev, val);
	}
	else if (cmd == "login")//登录
	{
		server_login(bev, val);
	}
	else if (cmd == "add")//添加好友
	{
		server_add_friend(bev, val);
	}
	else if (cmd == "create_group")//创建群聊
	{
		server_create_group(bev, val);
	}
	else if (cmd == "add_group")//添加群聊
	{
		server_add_group(bev, val);
	}
	else if (cmd == "private_chat")//私聊
	{
		server_private_chat(bev, val);
	}
	else if (cmd == "group_chat")//群聊
	{
		server_group_chat(bev, val);
	}
	else if (cmd == "get_group_member")//获取群成员
	{
		server_get_group_member(bev, val);
	}
	else if (cmd == "offline")//下线
	{
		server_offline(bev, val);
	}
	else if (cmd == "send_file")//发送文件
	{
		server_send_file(bev, val);
	}

	cout << buf << endl;
}


void Server::event_cb(struct bufferevent* bev, short what, void* ctx)
{

}


void Server::server_register(struct bufferevent* bev, Json::Value val)
{
	chatdb->my_database_connect("user");
	
	if (chatdb->my_database_user_exist(val["user"].asString()))
	{
		Json::Value resval;
		resval["cmd"] = "register_reply";
		resval["result"] = "failure";

		string jsonStr = JsonToString(resval);
		reply_client(bev, jsonStr);
	}
	else
	{
		chatdb->my_database_user_add(val["user"].asString(), val["password"].asString());

		Json::Value resval;
		resval["cmd"] = "register_reply";
		resval["result"] = "success";

		string jsonStr = JsonToString(resval);
		reply_client(bev, jsonStr);
	}

	chatdb->my_database_disconnect();
}

void Server::server_login(struct bufferevent* bev, Json::Value val)
{
	chatdb->my_database_connect("user");
	if (!chatdb->my_database_user_exist(val["user"].asString()))
	{
		Json::Value resval;
		resval.clear();
		resval["cmd"] = "login_reply";
		resval["result"] = "user_not_exist";

		string jsonStr = JsonToString(resval);
		reply_client(bev, jsonStr);
		cout << jsonStr << endl;
		return;
	}
	if (!chatdb->my_database_user_password_correct(val["user"].asString(), val["password"].asString()))
	{
		Json::Value resval;
		resval["cmd"] = "login_reply";
		resval["result"] = "password_error";

		string jsonStr = JsonToString(resval);
		reply_client(bev, jsonStr);
		return;
	}

	User u;
	u.name = val["user"].asString();
	u.bev = bev;
	chatlist->online_user->push_back(u);

	string fri;
	string group;
	chatdb->my_database_get_friend_group(val["user"].asString(), fri, group);
	Json::Value resval;
	resval["cmd"] = "login_reply";
	resval["result"] = "success";
	resval["friend"] = fri;
	resval["group"] = group;
	string jsonStr = JsonToString(resval);
	reply_client(bev, jsonStr);

	/*int start = 0;
	int pos = 0;*/
	//while (1)
	//{
	//	int pos = fri.find('|', start);
	//	if (pos == -1)
	//	{
	//		break;
	//	}
	//	string name = fri.substr(start, pos - start);
	//	for (auto iter = chatlist->online_user->begin(); iter != chatlist->online_user->end(); iter++)
	//	{
	//		if (name == iter->name)
	//		{
	//			Json::Value resval;
	//			resval["cmd"] = "friend_login";
	//			resval["friend"] = val["user"].asString().c_str();

	//			string jsonStr = JsonToString(resval);
	//			reply_client(bev, jsonStr);
	//		}
	//	}
	//	start = pos + 1;
	//}
	//string name = fri.substr(start, fri.size() - start);
	//for (auto iter = chatlist->online_user->begin(); iter != chatlist->online_user->end(); iter++)
	//{
	//	if (name == iter->name)
	//	{
	//		Json::Value resval;
	//		resval["cmd"] = "friend_login";
	//		resval["friend"] = val["user"].asString().c_str();
	//		string jsonStr = JsonToString(resval);
	//		reply_client(bev, jsonStr);
	//	}
	//}


	int start = 0, end = 0, flag = 1;
	string name;
	Json::Value resv;
	while (flag)
	{
		end = fri.find('|', start);
		if (-1 == end)
		{
			name = fri.substr(start, fri.size() - start);
			flag = 0;
		}
		else
		{
			name = fri.substr(start, end - start);
		}

		for (auto iter = chatlist->online_user->begin();
			iter != chatlist->online_user->end(); iter++)
		{
			if (name == iter->name)
			{
				resv.clear();
				resv["cmd"] = "friend_login";
				resv["friend"] = val["user"].asString().c_str();;
				string jsonStr = JsonToString(resv);
				reply_client(iter->bev, jsonStr);
			}
		}
		start = end + 1;
	}

	chatdb->my_database_disconnect();
}

void Server::server_add_friend(struct bufferevent* bev, Json::Value val)
{
	chatdb->my_database_connect("user");
	if (!chatdb->my_database_user_exist(val["friend"].asString()))
	{
		Json::Value resval;
		resval.clear();
		resval["cmd"] ="add_reply";
		resval["result"] ="user_not_exist";

		string jsonStr = JsonToString(resval);
		reply_client(bev, jsonStr);
		cout << jsonStr << endl;
		return;
	}
	
	if (chatdb->my_database_is_friend(val["user"].asString(), val["friend"].asString()))
	{
		Json::Value resval;
		resval.clear();
		resval["cmd"] = "add_reply";
		resval["result"] = "already_friend";

		string jsonStr = JsonToString(resval);
		reply_client(bev, jsonStr);
		cout << jsonStr << endl;
		return;
	}
	
	//正式添加好友
	chatdb->my_database_add_new_friend(val["user"].asString(), val["friend"].asString());
	chatdb->my_database_add_new_friend(val["friend"].asString(), val["user"].asString());

	//通知客户端
	Json::Value resval1;
	resval1.clear();
	resval1["cmd"] = "add_reply";
	resval1["result"] = "success";
	resval1["friend"] = val["friend"];
	string jsonStr = JsonToString(resval1);
	reply_client(bev, jsonStr);
	cout << jsonStr << endl;

	for (auto iter=chatlist->online_user->begin();iter!= chatlist->online_user->end();iter++)
	{
		if (val["friend"]==iter->name)
		{
			Json::Value res1;
			res1["cmd"] = "add_friend_reply";
			res1["result"] = val["user"];
			string jsonStr = JsonToString(res1);
			reply_client(iter->bev, jsonStr);
			cout << jsonStr << endl;
		}
	}

	chatdb->my_database_disconnect();
}

void Server::server_create_group(struct bufferevent* bev, Json::Value val)
{
	chatdb->my_database_connect("chatgroup");

	if (chatdb->my_database_group_exist(val["group"].asString()))
	{
		Json::Value resval;
		resval["cmd"] = "create_group_reply";
		resval["result"] = "group_exist";
		string jsonStr = JsonToString(resval);
		reply_client(bev, jsonStr);
		return;
	}
	//正式开始建群
	chatdb->my_database_add_new_group(val["group"].asString(), val["user"].asString());

	Json::Value resval;
	resval["cmd"] = "create_group_reply";
	resval["result"] = "success";
	string jsonStr = JsonToString(resval);
	reply_client(bev, jsonStr);

	chatdb->my_database_disconnect();
}

void Server::server_add_group(struct bufferevent* bev, Json::Value val)
{
	//是否已存在
	if (!chatlist->info_group_exist(val["group"].asString()))
	{
		Json::Value resval;
		resval["cmd"] = "add_group_reply";
		resval["result"] = "group_not_exist";
		string jsonStr = JsonToString(resval);
		reply_client(bev, jsonStr);
		return;
	}
	//用户是不是已经在群里面了
	if (chatlist->info_user_in_group(val["group"].asString(), val["user"].asString()))
	{
		Json::Value resval;
		resval["cmd"] = "add_group_reply";
		resval["result"] = "user_in_group";
		string jsonStr = JsonToString(resval);
		reply_client(bev, jsonStr);
		return;
	}

	//在用户自己的表里面添加群
	chatdb->my_database_connect("user");
	chatdb->my_database_user_add_group(val["user"].asString(), val["group"].asString());
	chatdb->my_database_disconnect();

	//在群里面添加用户
	chatdb->my_database_connect("chatgroup");
	chatdb->my_database_group_add_user(val["group"].asString(), val["user"].asString());
	chatdb->my_database_disconnect();

	//修改链表
	chatlist->info_group_add_user(val["group"].asString(), val["user"].asString());
	Json::Value resval;
	resval["cmd"] = "add_group_reply";
	resval["result"] = "success";
	string jsonStr = JsonToString(resval);
	reply_client(bev, jsonStr);
}


void Server::server_private_chat(struct bufferevent* bev, Json::Value val)
{
	struct bufferevent* to_bev=NULL;
	chatlist->info_get_friend_bev(val["user_to"].asString(), to_bev);
	if (NULL == to_bev)
	{
		Json::Value resval;
		resval["cmd"] = "private_chat_reply";
		resval["result"] = "offline";

		string jsonStr = JsonToString(resval);
		reply_client(bev, jsonStr);
		return;
	}
	string jsonStr;
	jsonStr = JsonToString(val);
	reply_client(to_bev, jsonStr);

	Json::Value resval1;
	resval1["cmd"] = "private_chat_reply";
	resval1["result"] = "success";

	jsonStr = JsonToString(resval1);
	reply_client(bev, jsonStr);
}

void Server::server_group_chat(struct bufferevent* bev, Json::Value val)
{
	for (auto it = chatlist->group_info->begin(); it != chatlist->group_info->end(); it++)
	{
		if (val["group"].asString() == it->name)
		{
			for (auto i = it->l->begin(); i != it->l->end(); i++)
			{
				struct bufferevent* to_bev = NULL;
				chatlist->info_get_friend_bev(val["user_to"].asString(), to_bev);
				if (to_bev != NULL)
				{
					string jsonStr = JsonToString(val);
					reply_client(to_bev, jsonStr);
				}
			}
		}
	}

	Json::Value v;
	v["cmd"] = "group_chat_reply";
	v["result"] = "success";

	string jsonStr = JsonToString(val);
	reply_client(bev, jsonStr);
}

void Server::server_get_group_member(struct bufferevent* bev, Json::Value val)
{
	string member = chatlist->info_get_group_member(val["group"].asString());

	Json::Value v;
	v["cmd"] = "get_group_member_reply";
	v["member"] = member;
	v["group"] = val["group"];

	string jsonStr = JsonToString(val);
	reply_client(bev, jsonStr);
}

void Server::server_offline(struct bufferevent* bev, Json::Value val)
{
	//从链表中删除用户
	for (auto iter = chatlist->online_user->begin();
		iter != chatlist->online_user->end(); iter++)
	{
		if (iter->name == val["user"].asString())
		{
			chatlist->online_user->erase(iter);
			--iter;
			break;
		}
	}

	chatdb->my_database_connect("user");

	//获取好友列表并且返回
	string friend_list, group_list;
	string name, s;
	Json::Value v;

	chatdb->my_database_get_friend_group(val["user"].asString(), friend_list, group_list);

	//向好友发送下线提醒
	int start = 0, end = 0, flag = 1;
	while (flag)
	{
		end = friend_list.find('|', start);
		if (-1 == end)
		{
			name = friend_list.substr(start, friend_list.size() - start);
			flag = 0;
		}
		else
		{
			name = friend_list.substr(start, end - start);
		}

		for (auto iter = chatlist->online_user->begin();
			iter != chatlist->online_user->end(); iter++)
		{
			if (name == iter->name)
			{
				v.clear();
				v["cmd"] = "friend_offline";
				v["friend"] = val["user"];
				string jsonStr = JsonToString(v);
				reply_client(iter->bev, jsonStr);
			}
		}
		start = end + 1;
	}

	chatdb->my_database_disconnect();
}

void Server::server_send_file(struct bufferevent* bev, Json::Value val)
{
	Json::Value v;
	string jsonStr;
	//如果对方不在线，返回offline
	struct bufferevent* to_bev = NULL;
	chatlist->info_get_friend_bev(val["to_user"].asString(), to_bev);
	if (NULL == to_bev)
	{
		v["cmd"] = "send_file_reply";
		v["result"] = "offline";
		jsonStr = JsonToString(v);
		reply_client(bev, jsonStr);
		return;
	}

	//启动新线程，创建文件服务器
	int port = 8080, from_fd = 0, to_fd = 0;
	thread send_file_thread(send_file_handler, val["length"].asInt(), port, &from_fd, &to_fd);
	send_file_thread.detach();

	//返回给发送客户端
	v.clear();
	v["cmd"] = "send_file_port_reply";
	v["result"] = port;
	jsonStr = JsonToString(v);
	reply_client(bev, jsonStr);
	
	//等待连接
	int count_time = 10;
	while (from_fd<=0)
	{
		count_time--;
		usleep(100000);
		if (count_time<=0)
		{
			pthread_cancel(send_file_thread.native_handle());//取消
			v.clear();
			v["cmd"] = "send_file_port_reply";
			v["result"] = "timeout";
			jsonStr = JsonToString(v);
			reply_client(bev, jsonStr);
			return;
		}
	}

	//返回给接受客户端
	v.clear();
	v["cmd"] = "send_file_port_reply";
	v["result"] = port;
	jsonStr = JsonToString(v);
	reply_client(to_bev, jsonStr);

	//等待连接
	count_time = 10;
	while (to_fd <= 0)
	{
		count_time--;
		usleep(100000);
		if (count_time <= 0)
		{
			pthread_cancel(send_file_thread.native_handle());//取消
			v.clear();
			v["cmd"] = "send_file_port_reply";
			v["result"] = "timeout";
			jsonStr = JsonToString(v);
			reply_client(bev, jsonStr);
			return;
		}
	}


}

