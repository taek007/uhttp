//
// Created by hackeris on 15-6-20.
//

#include <sys/socket.h>
#include <arpa/inet.h>
#include "fastcgi.h"
#include "mem.h"
#include <errno.h>
#include <unistd.h>
#include <stdio.h>


static char FCGI_HOST[20];
static unsigned short FCGI_PORT;

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

void load_fcgi_config(u_config* config){

    if(config == NULL){
        strcpy(FCGI_HOST, "127.0.0.1");
        FCGI_PORT = 9000;
    }else{
        strcpy(FCGI_HOST, config->fcgi_host);
        FCGI_PORT = config->fcgi_port;
    }
}

int cgi_handle_request(http_request *request, char* www_root) {

    int sock;
    struct sockaddr_in serv_addr;
    ssize_t str_len;
    size_t content_length_r;
    char msg[50];

    char status[] = "HTTP/1.1 200 OK"CRLF;
    char header[] = "Server: uhttp"CRLF;

    sock = socket(PF_INET, SOCK_STREAM, 0);

    if (sock < 0) {
        return errno;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(FCGI_HOST);
    serv_addr.sin_port = htons(FCGI_PORT);

    if (-1 == connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) {
        return errno;
    }

    FCGI_BeginRequestRecord begin_record;
    begin_record.header = makeHeader(FCGI_BEGIN_REQUEST, FCGI_REQUEST_ID,
                                     sizeof(begin_record.body), 0);
    begin_record.body = makeBeginRequestBody(FCGI_RESPONDER);

    str_len = write(sock, &begin_record, sizeof(begin_record));
    if (-1 == str_len) {
        return errno;
    }

    strcpy(msg, www_root);
    strcat(msg, request->path);

    char content_length_string[10] = { 0 };
    sprintf(content_length_string,"%d",request->content_length);
    char *params[][2] = {
            {"SCRIPT_FILENAME", msg},
            {"REQUEST_METHOD",  request->method ? "POST" : "GET"},
            {"CONTENT_TYPE", "application/x-www-form-urlencoded"},
            {"SERVER_SOFTWARE", "uhttp"},
            {"QUERY_STRING",    request->query},
            {"CONTENT_LENGTH",    content_length_string},
            {NULL, NULL}
    };

    FCGI_ContentRecord *params_record;
    size_t pack_size;
    params_record = pack_params(params, &pack_size);
    if(write(sock, params_record, pack_size) < 0){
        return errno;
    }
    mem_free(params_record);

    FCGI_Header empty_header = makeHeader(FCGI_PARAMS, FCGI_REQUEST_ID, 0,0);
    if(write(sock, &empty_header, sizeof(empty_header)) < 0){
        return errno;
    }

    size_t content_length, padding_length;
    if(request->method == METHOD_POST && request->content_length){
        FCGI_ContentRecord* content_record;
        content_length = request->content_length;
        padding_length = (content_length % 8) == 0 ? 0 : 8 - (content_length % 8);
        content_record = mem_alloc(sizeof(FCGI_ContentRecord) + content_length + padding_length);
        content_record->header = makeHeader(
                FCGI_STDIN, FCGI_REQUEST_ID, content_length, padding_length);
        memset(content_record->data,0,content_length+padding_length);
        memcpy(content_record->data,request->content, content_length);
        if(write(sock, content_record,
                 sizeof(FCGI_ContentRecord) + content_length + padding_length)<0){
            return errno;
        }
    }

    FCGI_Header stdin_header;
    stdin_header = makeHeader(FCGI_STDIN, FCGI_REQUEST_ID, 0, 0);
    if(write(sock, &stdin_header, sizeof(stdin_header)) < 0){
        return errno;
    }

    if(write(request->socket, status, strlen(status))<0){
        goto err;
    }
    if(write(request->socket, header, strlen(header))<0){
        goto err;
    }

    FCGI_Header response_header;
    char *message;
    while(1){
        str_len = read(sock, &response_header, sizeof(response_header));
        if (-1 == str_len) {
            return errno;
        }
        if (response_header.type == FCGI_STDOUT) {
            content_length_r = ((size_t) response_header.contentLengthB1 << 8)
                               + ((size_t) response_header.contentLengthB0)
                               + response_header.paddingLength;
            message = (char *) mem_alloc(content_length_r);
            read(sock, message, content_length_r);
        }
        else if (response_header.type == FCGI_STDERR) {
            content_length_r = ((size_t) response_header.contentLengthB1 << 8)
                               + ((size_t) response_header.contentLengthB0)
                               + response_header.paddingLength;
            message = (char *) mem_alloc(content_length_r);
            read(sock, message, content_length_r);
        }else{
            break;
        }
        if(write(request->socket, message, content_length_r)< 0){
            goto err;
        }
        mem_free(message);
    }

    close(sock);
    return 0;
    err:
    close(sock);
    return errno;
}


