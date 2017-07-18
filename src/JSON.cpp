#include "JSON.h"

CJSON::CJSON()
{
	this->init();
}


CJSON::~CJSON()
{
	
}

int CJSON::init()
{
	data = "";
	data += "{";
	return 0;
}

int CJSON::push(const string& id, const string& value)
{
	this->data += '"' + id + '"' + ":" + '"' + value + '"' + ",";
	return 0;
}

string CJSON::pop()
{
	this->data.pop_back();
	this->data.push_back('}');

	return this->data;
}
