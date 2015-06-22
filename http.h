//
// Created by hackeris on 15-6-20.
//

#ifndef UHTTP_HTTP_H
#define UHTTP_HTTP_H

#define METHOD_GET  0
#define METHOD_POST 1

#define MIN(x, y) ((x)<(y)?(x):(y))

#define CRLF "\r\n"

#include <sys/types.h>

typedef struct _http_request_head {
    int pair_count;
    char **pairs;
    char *buffer;
} http_request_head;

typedef struct _http_request {
    unsigned int version;
    int method;
    char *path;
    char *query;
    http_request_head head;
    char *content;
    size_t content_length;
    int socket;
} http_request;

http_request *wrap_http_request(int coming_socket, http_request *request);

char *get_http_content(char *start, http_request *request);

unsigned int get_http_version(char *ver);

char *get_request_head(char *buffer, http_request_head *head);

char *get_request_line(char *buffer, http_request *request);

char *get_request_head(char *head_start, http_request_head *head);

char *get_head_value(http_request_head *head, const char *key, char *value, size_t n);

void free_http_request(http_request *request);

#endif //UHTTP_HTTP_H
