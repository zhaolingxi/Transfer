#include"sql_server.h"

bool UseSqlCmdWithLog(char* icmd, MYSQL*& mysql)
{
	if (!icmd || !mysql)
	{
		return false;
	}
	if (mysql_query(mysql, icmd) != 0)
	{
		cout << "mysql_query fail:" << icmd << endl;
		return false;
	}
	else
	{
		return true;
	}
}