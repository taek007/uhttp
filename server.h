//
// Created by hackeris on 15-6-20.
//

#ifndef UHTTP_SERVER_H
#define UHTTP_SERVER_H

#include "http.h"

void handle_http_request(http_request* request);

int create_server_socket(unsigned short port);

void handle_coming_socket(void *_sock);

void start_http_server();

int catHTML(int sock, char* path);

#endif //UHTTP_SERVER_H
