//
// Created by hackeris on 15-6-21.
//

#ifndef UHTTP_CONF_H
#define UHTTP_CONF_H

#define CONF_IPADDRESS_KEY  "address"
#define CONF_PORT_KEY       "port"
#define CONF_WEBROOT_KEY    "webroot"

typedef struct _u_config{
    int ip_address;
    unsigned short port;
    char* web_root;
}u_config;

u_config* load_config_file(char* file);

char* load_config_string(char* file);

void free_config_string(char* str);

u_config *parse_config(char* conf_string);

void free_config(u_config* conf);

#endif //UHTTP_CONF_H
