#pragma once

#include <stdio.h>
#include <curl/curl.h>
/*  ubuntuでcurlを使うには、下記でインストールする :
    参考 url : https://curl.haxx.se/libcurl/c/
    sudo apt-get install curl libcurl3 libcurl3-dev
    includeディレクトリは /usr/include/curl/curl.h
    コンパイル時に　-lcurl と書くと良い                    */

class CPost_curl{

public:
    CPost_curl();
    ~CPost_curl();
    int Begin(const char *curl_url);
    int send_post(const char *post_data);

private:
    CURL *curl;

    static size_t receive_post_data(void* ptr, size_t size, size_t nmemb, void* data);
};


