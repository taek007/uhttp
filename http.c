//
// Created by hackeris on 15-6-20.
//

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "http.h"
#include "mem.h"
#include "log.h"

http_request *wrap_http_request(int coming_socket, http_request *request) {

    char buf[1024];
    ssize_t len = read(coming_socket, buf, sizeof(buf) - 1);
    buf[len] = '\0';

    if (NULL == strstr(buf, "HTTP/")) {
        return NULL;
    }

    char *token_remain = buf;

    request->socket = coming_socket;

    token_remain = get_request_line(token_remain, request);

    token_remain = get_http_head(token_remain, request);

    token_remain += 4;  //  skip 2 \r\n

    get_http_content(token_remain, request);

    return request;
}

char* get_request_line(char* buffer, http_request* request){

    char method[10];
    char path[250];
    char version[20];

    strcpy(method, strsep(&buffer, " "));
    strcpy(path, strsep(&buffer, " "));
    strcpy(version, strsep(&buffer, "\r\n"));

    logoutf("%s %s %s\n",method, path, version);

    request->path = strdup(path);

    if (strcmp(method, "GET") == 0) {
        request->method = METHOD_GET;
    } else if (strcmp(method, "POST") == 0) {
        request->method = METHOD_POST;
    }
    request->version = get_http_version(version);

    return buffer + 1;  //  skip the \n at last
}

char* get_http_content(char* start, http_request* request){

    char content_length_string[10];
    if (get_head_value(request->head, "Content-Length",
                       content_length_string, sizeof(content_length_string - 1))) {
        size_t content_length;
        sscanf(content_length_string, "%d", &content_length);
        request->content = mem_alloc(content_length);
        memcpy(request->content, start, content_length);

        logoutf("%s\n", request->content);
        return request->content;
    }
    request->content = NULL;
    return NULL;
}

char* get_http_head(char*token_remain, http_request* request){

    size_t head_size = 0;

    char *head_start = token_remain;
    char *head_end = strstr(token_remain, "\r\n\r\n");
    if (head_end == NULL) {
        head_size = strlen(head_start);
    } else {
        head_size = head_end - head_start;
    }

    request->head = mem_alloc(head_size + 1);
    strncpy(request->head, head_start, head_size);
    return head_start + head_size;
}


unsigned int get_http_version(char *ver) {

    unsigned int lo, hi;
    sscanf(ver, "HTTP/%d.%d", &hi, &lo);
    return ((hi << 8) | lo);
}


char *get_head_value(char *head, const char *key, char *value, int n) {

    char *start = strstr(head, key);
    if (start == NULL) {
        return NULL;
    }
    start += strlen(key) + 1;
    char *end = strstr(start, "\r\n");
    if (end == NULL) {
        return NULL;
    }
    return strncpy(value, start, MIN(n, end - start));
}

void free_http_request(http_request* request) {

    if(request->head != NULL){
        mem_free(request->head);
        request->head = NULL;
    }
    if(request->content != NULL){
        mem_free(request->content);
        request->content = NULL;
    }
    if(request->path != NULL){
        mem_free(request->path);
        request->path = NULL;
    }
    close(request->socket);
}
