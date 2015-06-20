#include <stdio.h>
#include <fcntl.h>
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


int main(int argc, char *argv[]) {

    int server_socket, client_socket;
    struct sockaddr_in server_addr;
    int opt = 1;

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("creante socket error %s(errno: %d)\n", strerror(errno), errno);
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(8000);

    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(server_socket, (struct sockaddr *) &server_addr,
             sizeof(server_addr)) == -1) {
        printf("bind socket error: %s(errno: %d)\n", strerror(errno), errno);
        exit(2);
    }

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

        http_request request;
        wrap_http_request(client_socket, &request);
        handle_http_request(&request);
    }

    close(server_socket);

    return 0;
}


