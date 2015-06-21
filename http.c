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

    token_remain = get_request_head(token_remain, &request->head);

    token_remain += 4;  //  skip 2 \r\n

    get_http_content(token_remain, request);

    return request;
}

char* get_request_line(char* buffer, http_request* request){

    char method[10];
    char path[1024];
    char version[20];
    char *query;

    strcpy(method, strsep(&buffer, " "));
    strcpy(path, strsep(&buffer, " "));
    strcpy(version, strsep(&buffer, "\r\n"));
    query = index(path, '?');
    if(query != NULL){
        *query = '\0';
        query++;
        request->query = strdup(query);
    }

    logoutf("%s %s %s\r\n",method, path, version);

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
    if (get_head_value(&request->head, "Content-Length",
                       content_length_string, sizeof(content_length_string - 1))) {
        size_t content_length;
        sscanf(content_length_string, "%d", &content_length);
        request->content = mem_alloc(content_length + 1);
        request->content[content_length] = '\0';
        memcpy(request->content, start, content_length);

        if(request->method==METHOD_POST){
            request->query = strdup(request->content);
        }

        return request->content;
    }
    request->content = NULL;
    return NULL;
}

char*get_request_head(char *head_start, http_request_head *head){

    char* head_end = strstr(head_start, "\r\n\r\n");
    char* start = mem_alloc(head_end - head_start + 1);
    char* end = start + (head_end - head_start) + 2;
    strncpy(start, head_start, head_end-head_start);
    start[head_end-head_start] = 0;
    head->buffer = start;
    int i;

    head->pair_count = 0;

    for(i=0; i < end - start - 1; i++){
        if(start[i] == '\r' && start[i+1] == '\n'){
            i++;
            head->pair_count ++;
        }
    }

    head->pairs = mem_alloc(sizeof(char*)*head->pair_count);

    int j = 0;
    i = 0;
    head->pairs[i] = start;
    for(i= 0; i < end - start - 2; i++){
        if(start[i] == '\r' && start[i+1] == '\n'){
            start[i] = start[i+1] = '\0';
            head->pairs[j++] = start + i + 2;
            i++;
        }
    }
    return head_end;
}

unsigned int get_http_version(char *ver) {

    unsigned int lo, hi;
    sscanf(ver, "HTTP/%d.%d", &hi, &lo);
    return ((hi << 8) | lo);
}

char* get_head_value(http_request_head *head, const char *key, char *value, size_t n){

    int i;
    size_t key_len = strlen(key);
    for(i=0; i< head->pair_count; i++){
        if(strstr(head->pairs[i], key) == head->pairs[i]
           && head->pairs[i][key_len] == ':'){
            size_t len = MIN(n - 1, strlen(&head->pairs[i][key_len+2]));
            strncpy(value,
                    &head->pairs[i][key_len+2],
                    len);
            value[len] = '\0';
            return &head->pairs[i][key_len+2];
        }
    }
    return NULL;
}

void free_http_request_head(http_request_head* head){

    if(head->pair_count != 0){
        head->pair_count = 0;
        mem_free(head->pairs);
        mem_free(head->buffer);
    }
}

void free_http_request(http_request* request) {

    free_http_request_head(&request->head);

    if(request->content != NULL){
        mem_free(request->content);
        request->content = NULL;
    }
    if(request->path != NULL){
        mem_free(request->path);
        request->path = NULL;
    }
    if(request->query != NULL){
        mem_free(request->query);
        request->query = NULL;
    }
    close(request->socket);
}

