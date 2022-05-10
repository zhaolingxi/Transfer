#include "chatlist.h"



ChatInfo::ChatInfo()
{
	online_user=new list<user>;
	group_info=new list<group>;
	//在有人登录之后，第一时间去获取服务器上的群信息
	mydatabase=new ChatDataBase;
	mydatabase->my_database_connect("chatgroup");

	string groupname[MAXNUM];
	int group_num=mydatabase->my_database_get_group_name(groupname);
	
	//cout<<"num:"<<group_num<<endl;

	for(int i=0;i<group_num;i++)
	{
		//cout<<groupname[i]<<endl;
		group g;
		g.name=groupname[i];
		g.l=new list <GroupUser> ;
		
		string member;
		mydatabase->my_database_get_gorup_member(g.name,member);
		if (member.size()==0)
		{
			continue;
		}
		int start=0;
		GroupUser user;
		while(1)
		{
			int pos=member.find('|',start);
			if(pos==-1)
			{
				break;
			}
			user.name=member.substr(start,pos-start);
			g.l->push_back(user);
			start=pos+1;
			user.name.clear();
		}
		user.name=member.substr(start,member.size()-start);
		g.l->push_back(user);
		cout<<member<<endl;
		group_info->push_back(g);

	}
	mydatabase->my_database_disconnect();
	cout<<"初始化链表完成"<<endl;
//	auto iter=group_info->begin();
//	for(;iter!=group_info->end();iter++)
//	{
//		cout<<"g name:"<<iter->name<<endl;
//		auto iterin=iter->l->begin();
//		for(;iterin!=iter->l->end();iterin++)
//		{
//			 cout<<"u name:"<<iterin->name<<endl;
//		}
//	}

}

bool ChatInfo::info_group_exist(string groupname)
{
	for (auto iter= group_info->begin(); iter != group_info->end(); iter++)
	{
		if (iter->name== groupname)
		{
			return true;
		}
	}
	return false;
}

bool ChatInfo::info_user_in_group(string groupname, string username)
{
	for (auto iter = group_info->begin(); iter != group_info->end(); iter++)
	{
		if (iter->name == groupname)
		{
			for (auto iteruser = iter->l->begin(); iteruser != iter->l->end(); iteruser++)
			{
				if (username== iteruser->name)
				{
					return true;
				}
			}
		}
	}
	return false;
}


void ChatInfo::info_group_add_user(string groupname, string username)
{
	for (auto iter = group_info->begin(); iter != group_info->end(); iter++)
	{
		if (iter->name == groupname)
		{
			groupuser user;
			user.name = username;
			iter->l->push_back(user);
		}
	}
}