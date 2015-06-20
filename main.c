#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "http.h"


void handle_http_request(http_request* request){

    char response[] = "HTTP/1.1 200 OK"CRLF
            "Server: NHTTP"CRLF
            "Content-Type: text/html;"CRLF CRLF
            "<h1>It works!</h1>";

    send(request->socket, response, strlen(response), 0);

    free_http_request(request);
}


int create_server_socket(unsigned short port){

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


void handle_coming_socket(void *_sock){

    int client_socket = *(int*)&_sock;

    http_request request;
    wrap_http_request(client_socket, &request);
    handle_http_request(&request);
}


void start_http_server(){

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


int main(int argc, char *argv[]) {

    start_http_server();
    return 0;
}


