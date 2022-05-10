#pragma once

#include <json/json.h>
#include <jsoncpp/json/json.h>

#include<string.h>
#include <sstream>
#include <memory>

using namespace std;

std::string JsonToString(const Json::Value& root);
