//
// Created by hackeris on 15-6-20.
//

#ifndef UHTTP_HTTP_H
#define UHTTP_HTTP_H

#define METHOD_GET  0
#define METHOD_POST 1


#define MIN(x,y) ((x)<(y)?(x):(y))


#define CRLF "\r\n"


typedef struct _http_request {
    unsigned int version;
    int method;
    char* path;
    char* head;
    char* content;
    int socket;
} http_request;

http_request *wrap_http_request(int coming_socket, http_request *request);

char* get_http_content(char* start, http_request* request);

unsigned int get_http_version(char* ver);

char* get_request_line(char* buffer, http_request* request);

char* get_http_head(char*token_remain, http_request* request);

char* get_head_value(char* head,const char* key, char* value, int n);

void free_http_request(http_request* request);

#endif //UHTTP_HTTP_H
