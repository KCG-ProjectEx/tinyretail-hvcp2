#include "JSON.h"

CJSON::CJSON()
{
	this->initFlag = 0;
	this->init();
}


CJSON::~CJSON()
{
	
}

int CJSON::init()
{
	data = "";
	data += "{";
	this->initFlag = 1;
	
	return 0;
}

int CJSON::push(const string& id, const string& value)
{
	if(initFlag != 1){
		printf("JSON push err");
		return -1;
	}
	this->data += '"' + id + '"' + ":" + '"' + value + '"' + ",";
	
	return 0;
}

string CJSON::pop()
{
	if(initFlag != 1){
		printf("JSON pop err");
		return NULL;
	}
	
	this->data.pop_back();
	this->data.push_back('}');
	
	this->initFlag = 0;
    
    cout << this->data << endl;
	
	return this->data;
}
