//
// Created by hackeris on 15-6-20.
//

#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>

#include "server.h"
#include "fastcgi.h"
#include "mem.h"
#include "log.h"

static char LISTEN_ADDRESS[20];
static unsigned short PORT;
static char WWW_ROOT[250];
static char FCGI_HOST[20];
static unsigned short FCGI_PORT;

int running = 1;
int server_socket = -1;

void handle_http_request(http_request *request) {

    char response[] = "HTTP/1.1 200 OK"CRLF
            "Server: uhttp"CRLF
            "Content-Type: text/html;"CRLF CRLF
            "<h1>It works!</h1>";

    char *ext = strrchr(request->path, '.');

    if (ext != NULL) {

        ext += 1;
        if (0 == strcmp(ext, "html")
            || 0 == strcmp(ext, "css")
            || 0 == strcmp(ext, "js")) {

            cat_text_file(request->socket, request->path);
        } else if (0 == strcmp(ext, "jpg")
                   || 0 == strcmp(ext, "ico")
                   || 0 == strcmp(ext, "png")) {
            cat_binary_file(request->socket, request->path);
        } else if (0 == strcmp(ext, "php")) {
            cat_php_file(request);
        }
        else {
            send(request->socket, response, strlen(response), 0);
        }
    } else {
        send(request->socket, response, strlen(response), 0);
    }

    free_http_request(request);
}


int create_server_socket(in_addr_t address, unsigned short port) {

    int server_socket;
    struct sockaddr_in server_address;
    int opt = 1;

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("creante socket error %s(errno: %d)\n", strerror(errno), errno);
        exit(1);
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = address;
    server_address.sin_port = htons(port);

    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(server_socket, (struct sockaddr *) &server_address,
             sizeof(server_address)) == -1) {
        printf("bind socket error: %s(errno: %d)\n", strerror(errno), errno);
        exit(2);
    }
    return server_socket;
}


void handle_coming_socket(void *_sock) {

    int client_socket = *(int *) &_sock;

    http_request request;
    memset(&request, 0, sizeof(request));
    wrap_http_request(client_socket, &request);
    handle_http_request(&request);
}

void fork_client_thread(int client_socket) {

    pthread_t thread;
    pthread_create(&thread, NULL, handle_coming_socket, client_socket);
    pthread_detach(thread);
}

void load_default_config(){

    strcpy(LISTEN_ADDRESS,"0.0.0.0");
    PORT = 8000;
    strcpy(WWW_ROOT, "/var/www");
    strcpy(FCGI_HOST, "127.0.0.1");
    FCGI_PORT = 9000;

    signal(SIGPIPE, SIG_IGN);
}

void load_server_config(u_config* config){

    load_default_config();
    if(config) {
        if(config->ip_address){
            strcpy(LISTEN_ADDRESS,config->ip_address);
        }
        if(config->port){
            PORT = config->port;
        }
        if(config->web_root){
            strcpy(WWW_ROOT, config->web_root);
        }
        if(config->fcgi_host) {
            strcpy(FCGI_HOST, config->fcgi_host);
        }
        if(config->fcgi_port){
            FCGI_PORT = config->fcgi_port;
        }
    }
}

void listen_socket(int server_socket){

    if (listen(server_socket, 10) == -1) {
        printf("listen socket error: %s(errno: %d)\n", strerror(errno), errno);
        exit(3);
    }
}

void server_main_loop(int server_socket){


    logoutf("waiting for clients.\n");
    while (running) {
        int client_socket;
        //	wait for client's connection.
        if ((client_socket = accept(server_socket, (struct sockaddr *) NULL,
                                    NULL)) == -1) {
            if (running) {
                //	if failed
                logoutf("accept socket error: %s(errno: %d)", strerror(errno),
                       errno);
            } else {
                logoutf("the uhttp server exited.\n");
            }
            break;
        }

        //handle_coming_socket(client_socket);
        fork_client_thread(client_socket);
    }
}

void start_http_server(u_config *config) {

    load_server_config(config);

    server_socket = create_server_socket(
            inet_addr(LISTEN_ADDRESS), PORT);

    listen_socket(server_socket);

    server_main_loop(server_socket);

    close(server_socket);
}

void stop_http_server() {

    running = 0;
    close(server_socket);
}

int cat_text_file(int sock, char *path) {

    char buf[1024];
    FILE *fp;

    char status[] = "HTTP/1.1 200 OK"CRLF;
    char header[] = "Server: uhttp"CRLF
            "Content-Type: text/html;"CRLF CRLF;

    char filepath[260];
    strcpy(filepath, WWW_ROOT);
    strcat(filepath, path);

    if(write(sock, status, strlen(status)) < 0){
        return errno;
    }
    if(write(sock, header, strlen(header)) < 0){
        return errno;
    }

    fp = fopen(filepath, "r");

    if (NULL == fp) {
        return 404;
    }

    fgets(buf, sizeof(buf), fp);
    while (!feof(fp)) {
        if(write(sock, buf, strlen(buf)) < 0){
            fclose(fp);
            return errno;
        }
        fgets(buf, sizeof(buf), fp);
    }

    fclose(fp);
}

int cat_binary_file(int sock, char *path) {

    char buf[1024];
    FILE *fp;
    ssize_t n;

    char status[] = "HTTP/1.1 200 OK"CRLF;
    char header[] = "Server: uhttp"CRLF
            "Content-Type: image/jpeg;"CRLF CRLF;

    char filepath[260];
    strcpy(filepath, WWW_ROOT);
    strcat(filepath, path);

    if(write(sock, status, strlen(status)) < 0){
        return errno;
    }
    if(write(sock, header, strlen(header)) < 0){
        return errno;
    }

    fp = fopen(filepath, "rb");
    if (NULL == fp) {
        return errno;
    }

    n = fread(buf, 1, sizeof(buf), fp);
    while (!feof(fp)) {
        if(write(sock, buf, n) < 0){
            fclose(fp);
            return errno;
        }
        n = fread(buf, 1, sizeof(buf), fp);
    }

    fclose(fp);
}

int cat_php_file(http_request *request) {

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

    strcpy(msg, WWW_ROOT);
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


