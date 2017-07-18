#ifndef Post_curl_H__
#define Post_curl_H__

#include <stdio.h>
#include <curl/curl.h>
/*  ubuntuでcurlを使うには、下記でインストールする :
    sudo apt-get install curl libcurl3 libcurl3-dev
    includeディレクトリは /usr/include/curl/curl.h         */

class Post_curl{

public:
    Post_curl();
    ~Post_curl();
    int Begin(const char *curl_url);
    int send_post(const char *post_data);

private:
    CURL *curl;
};

#endif