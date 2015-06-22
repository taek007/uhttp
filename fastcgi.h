//
// Created by hackeris on 15-6-20.
//

#ifndef UHTTP_FASTCGI_H
#define UHTTP_FASTCGI_H

#include <sys/types.h>
#include <string.h>
#include "http.h"
#include "conf.h"

#define FCGI_REQUEST_ID  1
#define FCGI_VERSION_1 1
#define FCGI_BEGIN_REQUEST 1
#define FCGI_RESPONDER 1
#define FCGI_END_REQUEST 3
#define FCGI_PARAMS 4
#define FCGI_STDIN 5
#define FCGI_STDOUT 6
#define FCGI_STDERR 7

/*
 * Fast_Head struct
 */
typedef struct{
    unsigned char version;
    unsigned char type;
    unsigned char requestIdB1;
    unsigned char requestIdB0;
    unsigned char contentLengthB1;
    unsigned char contentLengthB0;
    unsigned char paddingLength;
    unsigned char reserved;
}FCGI_Header;


typedef struct{
    unsigned char roleB1;
    unsigned char roleB0;
    unsigned char flags;
    unsigned char reserved[5];
}FCGI_BeginRequestBody;

typedef struct{
    FCGI_Header header;
    FCGI_BeginRequestBody body;
}FCGI_BeginRequestRecord;

typedef struct{
    FCGI_Header header;
    unsigned char data[0];
}FCGI_ContentRecord;

/*
 * 构造请求头部，返回FCGI_Header结构体
 */
FCGI_Header makeHeader(
        int type,
        int requestId,
        int contentLength,
        int paddingLength);

/*
 * 构造请求体，返回FCGI_BeginRequestBody结构体
 */
FCGI_BeginRequestBody makeBeginRequestBody(
        int role);

void load_fcgi_config(u_config* config);

FCGI_ContentRecord* pack_params(char *params[][2], size_t* pack_size);

int cgi_handle_request(http_request *request, char* www_root);

#endif //UHTTP_FASTCGI_H
