#include "Post_curl.h"

/*****************************************
@ function
@ parameter
@ return
******************************************/
Post_curl::Post_curl(){

    fprintf(stdout,"Welcome to curl\n");    
}

/*****************************************
@ function
    libcurlで使用したハンドルの開放
@ parameter
@ return
******************************************/
Post_curl::~Post_curl(){

    // curl_easy_init()で取得したハンドルを開放する
    // 呼び出し後のハンドルの使用は違反となる
    curl_easy_cleanup(curl);

    // libcurlの使用を終えた後に呼び出す
    curl_global_cleanup();

    fprintf(stdout,"See you curl\n");
}

/*****************************************
@ function
    libcurlの初期設定,
    ハンドルの取得,
@ parameter
    POSTの送信先url
@ return
    正常終了 0,　異常終了 -1
******************************************/
int Post_curl::Begin(const char *curl_url){

    /* In windows, this will init the winsock stuff */ 
    // グローバルlibcurlの初期化
    int ret = curl_global_init(CURL_GLOBAL_ALL);
    if( ret != 0)return -1;

    // ハンドルを取得する
    // ハンドルは再利用することでパフォーマンス向上させる 
    CURL *p = curl_easy_init();
    if( p == NULL )return -1;


    /***正常にセットアップされた***/
    // libcurlで正規に使うハンドルへ代入する
    curl = p; 

    //POSTの送信先URLを設定する
    curl_easy_setopt(curl, CURLOPT_URL, curl_url);

    //レスポンスデータを扱う関数を設定する(コールバック関数の設定)
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, receive_post_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);

    return 0;
}
/*****************************************
@ function
    httpレスポンスデータを表示させない為の関数
@ parameter
    POSTのレスポンスデータ
@ return
    正常終了 : 実際に処理されたバイト数,　異常終了 : 0
******************************************/
size_t Post_curl::receive_post_data(void* ptr, size_t size, size_t nmemb, void* data){
    
    if (size * nmemb == 0) return 0;

    return size * nmemb;
}

/*****************************************
@ function
    POSTデータを送信する
@ parameter
    POSTデータ
@ return
    正常終了 0, 異常終了 -1
******************************************/
int Post_curl::send_post(const char *post_data){

    CURLcode res;

    //POSTデータを指定する
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);

    //POST要求の実行
    res = curl_easy_perform(curl);

    //エラーチェック
    if(res != CURLE_OK){
        fprintf(stderr, "curl_easy_perform() failed: %s\n",curl_easy_strerror(res));
        return -1;
    }

    return 0;
}
