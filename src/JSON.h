#ifndef JSON_H__
#define JSON_H__

#include <iostream>
#include <string>

using namespace std;

class CJSON
{
public:
	CJSON();
	~CJSON();
	int push(const string& id, const string& value);
	string pop();
	int init();
	string data;
	bool initFlag;
};

#endif
