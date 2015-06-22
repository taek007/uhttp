//
// Created by hackeris on 15-6-20.
//

#include "fastcgi.h"
#include "mem.h"

/*
 * 构造请求头部，返回FCGI_Header结构体
 */
FCGI_Header makeHeader(
        int type,
        int requestId,
        int contentLength,
        int paddingLength)
{
    FCGI_Header header;
    header.version = FCGI_VERSION_1;
    header.type             = (unsigned char) type;
    header.requestIdB1      = (unsigned char) ((requestId     >> 8) & 0xff);
    header.requestIdB0      = (unsigned char) ((requestId         ) & 0xff);
    header.contentLengthB1  = (unsigned char) ((contentLength >> 8) & 0xff);
    header.contentLengthB0  = (unsigned char) ((contentLength     ) & 0xff);
    header.paddingLength    = (unsigned char) paddingLength;
    header.reserved         =  0;
    return header;
}

/*
 * 构造请求体，返回FCGI_BeginRequestBody结构体
 */
FCGI_BeginRequestBody makeBeginRequestBody(
        int role)
{
    FCGI_BeginRequestBody body;
    body.roleB1 = (unsigned char) ((role >>  8) & 0xff);
    body.roleB0 = (unsigned char) (role         & 0xff);
    body.flags  = (unsigned char) 0;
    memset(body.reserved, 0, sizeof(body.reserved));
    return body;
}

FCGI_ContentRecord* pack_params(char *params[][2], size_t* pack_size){

    size_t content_length = 0;
    int i;
    for(i=0;params[i][0] != NULL; i++){

        if(params[i][1] == NULL){
            continue;
        }
        content_length += 2 + strlen(params[i][0]) + strlen(params[i][1]);
    }

    size_t padding_length = (content_length % 8) == 0 ? 0 : 8 - (content_length % 8);
    *pack_size = sizeof(FCGI_Header) + content_length + padding_length;
    FCGI_ContentRecord *record = mem_alloc(*pack_size);
    memset(record, 0, *pack_size);
    record->header = makeHeader(FCGI_PARAMS,
                                FCGI_REQUEST_ID, content_length, padding_length);
    unsigned char* curr = record->data;
    for(i=0;params[i][0] != NULL; i++) {

        if (params[i][1] == NULL) {
            continue;
        }
        size_t k_len = strlen(params[i][0]);
        size_t v_len = strlen(params[i][1]);
        curr[0] = (unsigned char)k_len;
        curr[1] = (unsigned char)v_len;
        strcat(&curr[2], params[i][0]);
        strcat(&curr[2], params[i][1]);
        curr += (k_len + v_len + 2);
    }
    return record;
}
