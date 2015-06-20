//
// Created by hackeris on 15-6-20.
//

#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "server.h"

#define WWW_ROOT "/home/hackeris"

void handle_http_request(http_request *request) {

    char response[] = "HTTP/1.1 200 OK"CRLF
            "Server: NHTTP"CRLF
            "Content-Type: text/html;"CRLF CRLF
            "<h1>It works!</h1>";

    char* ext = strrchr(request->path, '.');

    if(ext != NULL) {

        ext += 1;
        if (0 == strcmp(ext, "html")
            || 0 == strcmp(ext, "css")
            || 0 == strcmp(ext, "js")) {

            cat_text_file(request->socket, request->path);
        } else if (0 == strcmp(ext, "jpg")
                   || 0 == strcmp(ext, "ico")
                   || 0 == strcmp(ext, "png")){
            cat_binary_file(request->socket, request->path);
        }
        else {
            send(request->socket, response, strlen(response), 0);
        }
    }else{
        send(request->socket, response, strlen(response), 0);
    }

    free_http_request(request);
}


int create_server_socket(unsigned short port) {

    int server_socket;
    struct sockaddr_in server_address;
    int opt = 1;

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("creante socket error %s(errno: %d)\n", strerror(errno), errno);
        exit(1);
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
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


void start_http_server() {

    int server_socket = create_server_socket(8000);
    int client_socket;

    if (listen(server_socket, 10) == -1) {
        printf("listen socket error: %s(errno: %d)\n", strerror(errno), errno);
        exit(3);
    }

    printf("waiting for clients.\n");
    while (1) {
        //	wait for client's connection.
        if ((client_socket = accept(server_socket, (struct sockaddr *) NULL,
                                    NULL)) == -1) {
            //	if failed
            printf("accept socket error: %s(errno: %d)", strerror(errno),
                   errno);
            break;
        }

        handle_coming_socket(client_socket);
    }

    close(server_socket);
}

int cat_text_file(int sock, char *path) {

    char buf[1024];
    FILE *fp;

    char status[] = "HTTP/1.1 200 OK"CRLF;
    char header[] = "Server: NHTTP"CRLF
            "Content-Type: text/html;"CRLF CRLF;

    char filepath[260];
    strcpy(filepath, WWW_ROOT);
    strcat(filepath,path);

    write(sock, status, strlen(status));
    write(sock, header, strlen(header));

    fp = fopen(filepath, "r");

    if (NULL == fp) {
        return 404;
    }

    fgets(buf, sizeof(buf), fp);
    while (!feof(fp)) {
        write(sock, buf, strlen(buf));
        fgets(buf, sizeof(buf), fp);
    }

    fclose(fp);
}

int cat_binary_file(int sock, char *path){

    char buf[1024];
    FILE *fp;

    char status[] = "HTTP/1.1 200 OK"CRLF;
    char header[] = "Server: NHTTP"CRLF
            "Content-Type: image/jpeg;"CRLF CRLF;

    char filepath[260];
    strcpy(filepath, WWW_ROOT);
    strcat(filepath,path);

    write(sock, status, strlen(status));
    write(sock, header, strlen(header));

    fp = fopen(filepath, "rb");

    if (NULL == fp) {
        return 404;
    }

    size_t n = fread(buf, 1,sizeof(buf),fp);
    while (!feof(fp)) {
        write(sock, buf, n);
        n = fread(buf, 1,sizeof(buf),fp);
    }

    fclose(fp);
}
