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
}

void Server::listener_cb(struct evconnlistener* listener, evutil_socket_t fd, struct sockaddr* addr, int socklen, void* arg)
{
	cout << "accpet client connect" << fd << endl;

	//为每一个客户端创建一个线程，用于服务
	thread client_thread(client_handler, fd);

	client_thread.detach();
}

void Server::client_handler(int fd)
{
	struct event_base* base = event_base_new();

	struct bufferevent* bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);

	if (!bev)
	{
		cout << "bufferevent_socket_new error" << endl;
	}

	bufferevent_setcb(bev, read_cb, NULL, event_cb, NULL);

	bufferevent_enable(bev, EV_READ);

	event_base_dispatch(base);

	event_base_free(base);
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

	//JSONCPP_STRING errs;
	string errs;
	Json::Value val;
	Json::CharReaderBuilder readerBuilder;
	Json::Value def;
	def["emitUTF8"] = true;
	readerBuilder.setDefaults(&def);
	std::unique_ptr<Json::CharReader> const jsonReader(readerBuilder.newCharReader());

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
		server_add_group(bev, val);
	}
	else if (cmd == "add_group")//添加群聊
	{
	}
	else if (cmd == "private_chat")//私聊
	{
	}
	else if (cmd == "group_chat")//群聊
	{
	}
	else if (cmd == "add_group")//添加群
	{
	}
	else if (cmd == "get_group_member")//获取群成员
	{
	}
	else if (cmd == "offline")//下线
	{
	}
	else if (cmd == "send_file")//发送文件
	{
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
		cout << jsonStr << endl;

		if (bufferevent_write(bev, jsonStr.c_str(), jsonStr.length() )< 0)
		{
			cout << "bufferevent_write fail" << endl;
		}
	}
	else
	{
		chatdb->my_database_user_add(val["user"].asString(), val["password"].asString());

		Json::Value resval;
		resval["cmd"] = "register_reply";
		resval["result"] = "success";

		string jsonStr = JsonToString(resval);
		cout << jsonStr << endl;

		if (bufferevent_write(bev, jsonStr.c_str(), jsonStr.length()) < 0)
		{
			cout << "bufferevent_write fail" << endl;
		}
	}

	chatdb->my_database_disconnect();
}

void Server::server_login(struct bufferevent* bev, Json::Value val)
{
	chatdb->my_database_connect("user");
	if (!chatdb->my_database_user_exist(val["user"].asString()))
	{
		cout << 1 << endl;
		Json::Value resval;
		resval["cmd"] = "login_reply";
		resval["result"] = "usernotexist";

		std::ostringstream os;
		Json::StreamWriterBuilder writerBuilder;
		Json::Value def;
		def["emitUTF8"] = true;
		writerBuilder.setDefaults(&def);
		std::unique_ptr<Json::StreamWriter> jsonWriter(writerBuilder.newStreamWriter());
		jsonWriter->write(resval, &os);
		string jsonStr = os.str();

		if (bufferevent_write(bev, jsonStr.c_str(), jsonStr.length()) < 0)
		{
			cout << "bufferevent_write fail" << endl;
		}
		return;
	}
	if (!chatdb->my_database_user_password_correct(val["user"].asString(), val["password"].asString()))
	{
		cout << 2 << endl;
		Json::Value resval;
		resval["cmd"] = "login_reply";
		resval["result"] = "usernotexist";

		std::ostringstream os;
		Json::StreamWriterBuilder writerBuilder;
		Json::Value def;
		def["emitUTF8"] = true;
		writerBuilder.setDefaults(&def);
		std::unique_ptr<Json::StreamWriter> jsonWriter(writerBuilder.newStreamWriter());
		jsonWriter->write(resval, &os);
		string jsonStr = os.str();

		if (bufferevent_write(bev, jsonStr.c_str(), jsonStr.length()) < 0)
		{
			cout << "bufferevent_write fail" << endl;
		}
		return;
	}

	User u;
	u.name = val["user"].asString();
	u.bev = bev;
	chatlist->online_user->push_back(u);

	cout << 3 << endl;

	string fri;
	string group;
	chatdb->my_database_get_friend_group(val["user"].asString(), fri, group);

	cout << 4 << endl;
	cout << fri << endl;

	Json::Value resval;
	resval["cmd"] = "login_reply";
	resval["result"] = "success";
	resval["friend"] = fri;
	resval["group"] = group;
	string jsonStr = JsonToString(resval);
	cout << jsonStr << endl;
	if (bufferevent_write(bev, jsonStr.c_str(), jsonStr.length()) < 0)
	{
		cout << "bufferevent_write fail" << endl;
	}

	int start = 0;
	int pos = 0;
	while (1)
	{
		int pos = fri.find('|', start);
		if (pos == -1)
		{
			break;
		}
		string name = fri.substr(start, pos - start);
		for (auto iter = chatlist->online_user->begin(); iter != chatlist->online_user->end(); iter++)
		{
			if (name == iter->name)
			{
				Json::Value resval;
				resval["cmd"] = "friend_login";
				resval["friend"] = val["user"].asString().c_str();

				string jsonStr = JsonToString(resval);
				cout << jsonStr << endl;
				if (bufferevent_write(bev, jsonStr.c_str(), jsonStr.length()) < 0)
				{
					cout << "bufferevent_write fail" << endl;
				}
			}
		}
		start = pos + 1;
	}
	string name = fri.substr(start, fri.size() - start);
	for (auto iter = chatlist->online_user->begin(); iter != chatlist->online_user->end(); iter++)
	{
		if (name == iter->name)
		{
			Json::Value resval;
			resval["cmd"] = "friend_login";
			resval["friend"] = val["user"].asString().c_str();
			string jsonStr = JsonToString(resval);
			cout << jsonStr << endl;
			if (bufferevent_write(bev, jsonStr.c_str(), jsonStr.length()) < 0)
			{
				cout << "bufferevent_write fail" << endl;
			}
		}
	}

	chatdb->my_database_disconnect();
}

void Server::server_add_friend(struct bufferevent* bev, Json::Value val)
{
	chatdb->my_database_connect("user");
	if (!chatdb->my_database_user_exist(val["friend"].asString()))
	{
		Json::Value resval;
		resval["cmd"] = "add_reply";
		resval["result"] = "user_not_exist";

		string jsonStr = JsonToString(resval);
		cout << jsonStr << endl;

		if (bufferevent_write(bev, jsonStr.c_str(), jsonStr.length() )< 0)
		{
			cout << "bufferevent_write fail" << endl;
		}
		return;
	}
	
	if (chatdb->my_database_is_friend(val["user"].asString(), val["friend"].asString()))
	{
		Json::Value resval;
		resval["cmd"] = "add_reply";
		resval["result"] = "already_friend";

		string jsonStr = JsonToString(resval);
		cout << jsonStr << endl;

		if (bufferevent_write(bev, jsonStr.c_str(), jsonStr.length()) < 0)
		{
			cout << "bufferevent_write fail" << endl;
		}
		return;
	}
	
	//正式添加好友
	chatdb->my_database_add_new_friend(val["user"].asString(), val["friend"].asString());
	chatdb->my_database_add_new_friend(val["friend"].asString(), val["user"].asString());

	//通知客户端
	Json::Value resval1;
	resval1["cmd"] = "add_reply";
	resval1["result"] = "success";
	string jsonStr = JsonToString(resval1);
	cout << jsonStr << endl;
	if (bufferevent_write(bev, jsonStr.c_str(), jsonStr.length()) < 0)
	{
		cout << "bufferevent_write fail" << endl;
	}

	for (auto iter=chatlist->online_user->begin();iter!= chatlist->online_user->end();iter++)
	{
		if (val["friend"]==iter->name)
		{
			Json::Value res1;
			res1["cmd"] = "add_friend_reply";
			res1["result"] = val["user"];
			string jsonStr = JsonToString(res1);
			cout << jsonStr << endl;
			if (bufferevent_write(bev, jsonStr.c_str(), jsonStr.length()) < 0)
			{
				cout << "bufferevent_write fail" << endl;
			}
		}
	}

	chatdb->my_database_disconnect();
}

void Server::server_create_group(struct bufferevent* bev, Json::Value val)
{}

void Server::server_add_group(struct bufferevent* bev, Json::Value val)
{
	chatdb->my_database_connect("chatgroup");

	if (chatdb->my_database_group_exist(val["group"].asString()))
	{
		Json::Value resval;
		resval["cmd"] = "create_group_reply";
		resval["result"] = "group_exist";
		string jsonStr = JsonToString(resval);
		cout << jsonStr << endl;
		if (bufferevent_write(bev, jsonStr.c_str(), jsonStr.length()) < 0)
		{
			cout << "bufferevent_write fail" << endl;
		}
		return;
	}
	//正式开始建群
	chatdb->my_database_add_new_group(val["group"].asString(), val["user"].asString());

	Json::Value resval;
	resval["cmd"] = "create_group_reply";
	resval["result"] = "success";
	string jsonStr = JsonToString(resval);
	cout << jsonStr << endl;
	if (bufferevent_write(bev, jsonStr.c_str(), jsonStr.length()) < 0)
	{
		cout << "bufferevent_write fail" << endl;
	}

	chatdb->my_database_disconnect();
}


void Server::server_private_chat(struct bufferevent* bev, Json::Value val)
{}

void Server::server_group_chat(struct bufferevent* bev, Json::Value val)
{}

void Server::server_get_group_member(struct bufferevent* bev, Json::Value val)
{}

void Server::server_offline(struct bufferevent* bev, Json::Value val)
{}

void Server::server_send_file(struct bufferevent* bev, Json::Value val)
{}

