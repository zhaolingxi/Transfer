#pragma once

#include<mysql/mysql.h>

#include<string.h>
#include <sstream>
#include <iostream>

using namespace std;

bool UseSqlCmdWithLog(char * icmd, MYSQL* &mysql);
