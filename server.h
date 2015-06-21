//
// Created by hackeris on 15-6-20.
//

#ifndef UHTTP_SERVER_H
#define UHTTP_SERVER_H

#include <arpa/inet.h>

#include "http.h"
#include "conf.h"

void handle_http_request(http_request* request);

int create_server_socket(in_addr_t address,unsigned short port);

void handle_coming_socket(void *_sock);

void start_http_server(u_config* config);

void stop_http_server();

int cat_text_file(int sock, char *path);

int cat_binary_file(int sock, char *path);

int cat_php_file(http_request* request);

#endif //UHTTP_SERVER_H
