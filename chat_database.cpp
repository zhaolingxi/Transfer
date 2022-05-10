#include"chat_database.h"
#include"sql_server.h"

ChatDataBase::ChatDataBase()
{
	mysql = mysql_init(NULL);
}


ChatDataBase::~ChatDataBase()
{
	delete mysql;
}

void ChatDataBase::my_database_connect(string name)
{
	mysql = mysql_real_connect(mysql, "127.0.0.1", "root", "mysqlpwd", name.c_str(), 0, NULL, 0);
	if (NULL == mysql)
	{
		cout << "connect sql fail\n" << endl;
	}

	if (mysql_query(mysql, "set names utf8;") != 0)
	{
		cout << "mysql_query fail" << endl;
	}
	string str = "use ";
	str += name;
	str += ";";
	mysql_query(mysql, str.c_str());
}

void ChatDataBase::my_database_disconnect()
{
	mysql_close(mysql);
}

int ChatDataBase::my_database_get_group_name(string* s)
{
	if (mysql_query(mysql, "set names utf8;") != 0)
	{
		cout << "set names utf8 fail" << endl;
	}
	if (mysql_query(mysql, "show tables;") != 0)
	{
		cout << "mysql query fail" << endl;
	}
	MYSQL_RES* res = mysql_store_result(mysql);
	if (NULL == res)
	{
		cout << "mysql_store_result" << endl;
	}

	int count = 0;
	MYSQL_ROW row;
	while (row = mysql_fetch_row(res))
	{
		s[count] += row[0];
		count++;
	}
	return count;
}


void ChatDataBase::my_database_get_gorup_member(string name, string& s)
{
	char sql[1024] = { 0 };
	sprintf(sql, "select member from %s", name.c_str());
	if (mysql_query(mysql, sql) != 0)
	{
		cout << "mysql_query fail" << endl;
	}

	MYSQL_RES* res = mysql_store_result(mysql);
	if (NULL == res)
	{
		cout << "mysql_store_result" << endl;
	}

	MYSQL_ROW row = mysql_fetch_row(res);
	if (NULL == row[0])
	{
		return;
	}
	s += row[0];
}

bool ChatDataBase::my_database_user_exist(string name)
{
	if (mysql_query(mysql, "set names utf8;") != 0)
	{
		cout << "mysql_query fail" << endl;
	}

	char sql[128] = { 0 };
	sprintf(sql, "show tables like '%s';", name.c_str());
	if (mysql_query(mysql, sql) != 0)
	{
		cout << "mysql_query fail" << endl;
	}

	MYSQL_RES* res = mysql_store_result(mysql);
	if (NULL == res)
	{
		cout << "mysql_store_result" << endl;
	}

	MYSQL_ROW row = mysql_fetch_row(res);
	if (NULL == row)
	{
		return false;
	}
	else
	{
		return true;
	}
}

void ChatDataBase::my_database_user_add(string name, string password)
{
	char sql[128] = { 0 };
	sprintf(sql, "create table %s (password varchar(16),friend varchar(4096),chatgroup varchar(4096)) character set utf8;", name.c_str());
	if (mysql_query(mysql, sql) != 0)
	{
		cout << "mysql_query fail" << endl;
	}

	memset(sql, 0, strlen(sql));
	sprintf(sql, "insert into %s (password) values ('%s');", name.c_str(), password.c_str());
	if (mysql_query(mysql, sql) != 0)
	{
		cout << "mysql_query fail" << endl;
	}

}

bool ChatDataBase::my_database_user_password_correct(string name, string password)
{
	if (mysql_query(mysql, "set names utf8;") != 0)
	{
		cout << "mysql_query fail" << endl;
	}

	char sql[128] = { 0 };
	sprintf(sql, "select password from %s;", name.c_str());
	if (mysql_query(mysql, sql) != 0)
	{
		cout << "mysql_query fail" << endl;
	}

	MYSQL_RES* res = mysql_store_result(mysql);
	if (NULL == res)
	{
		cout << "mysql_store_result" << endl;
	}

	MYSQL_ROW row = mysql_fetch_row(res);
	if (row[0] == password)
	{
		return true;
	}
	else
	{
		return false;
	}
}

string ChatDataBase::my_database_get_friend(string name)
{
	char sql[128] = { 0 };
	sprintf(sql, "select friend from %s;", name.c_str());
	if (mysql_query(mysql, sql) != 0)
	{
		cout << "mysql_query fail" << endl;
	}
	MYSQL_RES* res = mysql_store_result(mysql);
	if (NULL == res)
	{
		cout << "mysql_store_result" << endl;
	}
	string result;
	MYSQL_ROW row = mysql_fetch_row(res);
	if (NULL == row[0])
	{
		return result;
	}
	else
	{
		result.append(row[0]);
		return result;
	}
}

bool ChatDataBase::my_database_is_friend(string name, string name2)
{
	char sql[128] = { 0 };
	sprintf(sql, "select friend from %s;", name.c_str());
	if (mysql_query(mysql, sql) != 0)
	{
		cout << "mysql_query fail" << endl;
	}
	MYSQL_RES* res = mysql_store_result(mysql);
	MYSQL_ROW row = mysql_fetch_row(res);
	if (NULL == row[0])
	{
		return false;
	}
	else
	{
		string all_friend(row[0]);
		cout << "all_friend:" << all_friend << endl;
		cout << "name:" << name << endl;
		cout<<"name2:"<< name2 << endl;
		int start = 0;
		int end = 0;
		while (1)
		{
			end = all_friend.find('|',start);
			if (-1==end)
			{
				break;
			}
			else if(name2==all_friend.substr(start,end-start))
			{
				cout << true << endl;
				return true;
			}
			start = end + 1;
			cout << end << endl;
		}
		if (name2 == all_friend.substr(start, all_friend.size() - start))
		{
			cout << true << endl;
			return true;
		}

		return false;
	}
}

void ChatDataBase::my_database_add_new_friend(string name, string namefri)
{
	char sql[1024] = { 0 };
	cout << "select friend from " << name<< endl;
	sprintf(sql, "select friend from %s;", name.c_str());
	if (mysql_query(mysql, sql) != 0)
	{
		cout << "mysql_query fail" << endl;
	}
	MYSQL_RES* res = mysql_store_result(mysql);
	string result;
	MYSQL_ROW row = mysql_fetch_row(res);
	if (NULL == row[0])
	{
		result.append(namefri);
	}
	else
	{
		result.append(row[0]);
		result += "|";
		result.append(namefri);
	}
	memset(sql, 0, sizeof(sql));
	sprintf(sql, "update %s set friend = '%s';", name.c_str(), result.c_str());
	if (mysql_query(mysql, sql) != 0)
	{
		cout << "mysql_query update fail" << endl;
	}
	
}

bool ChatDataBase::my_database_group_exist(string name)
{
	char sql[128] = { 0 };
	sprintf(sql, "show tables like '%s';", name.c_str());
	if (mysql_query(mysql, sql) != 0)
	{
		cout << "mysql_query fail" << endl;
	}

	MYSQL_RES* res = mysql_store_result(mysql);
	if (NULL == res)
	{
		cout << "mysql_store_result fail" << endl;
	}
	MYSQL_ROW row = mysql_fetch_row(res);
	if (NULL == row)
	{
		return false;
	}
	else
	{
		return true;
	}
}


void ChatDataBase::my_database_add_new_group(string groupname,string groupowner)
{
	char sql[128] = { 0 };
	sprintf(sql, "create table %s (owner varchar(32), member varchar(4096)) char set utf8;", groupname.c_str());
	if (mysql_query(mysql, sql) != 0)
	{
		cout << "mysql_query fail" << endl;
	}

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "insert into %s values ('%s', '%s') ;", groupname.c_str(), groupowner.c_str(), groupowner.c_str());
	if (mysql_query(mysql, sql) != 0)
	{
		cout << "mysql_query fail" << endl;
	}

}


void ChatDataBase::my_database_get_friend_group(string name, string& f, string& g)
{
	char sql[128] = { 0 };
	sprintf(sql, "select friend from %s;", name.c_str());
	UseSqlCmdWithLog(sql, mysql);

	MYSQL_RES* res = mysql_store_result(mysql);
	MYSQL_ROW row = mysql_fetch_row(res);
	if (row[0] != NULL)
	{
		f.append(row[0]);
	}
	mysql_free_result(res);

	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select chatgroup from %s;", name.c_str());
	UseSqlCmdWithLog(sql, mysql);

	res = mysql_store_result(mysql);
	row = mysql_fetch_row(res);
	if (row[0] != NULL)
	{
		g.append(row[0]);
	}
}