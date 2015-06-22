//
// Created by hackeris on 15-6-21.
//

#include <stdio.h>
#include <json/json.h>
#include <string.h>
#include "conf.h"
#include "mem.h"

#define FREE(p, f)    do{if(p)f(p);p=NULL;}while(0)


u_config* load_config_file(char* file){

    char* conf_string = load_config_string(file);
    if(NULL == conf_string){
        return NULL;
    }
    u_config* config = parse_config(conf_string);
    free_config_string(conf_string);
    return config;
}

char* load_config_string(char* file){

    FILE *fp = fopen(file, "r");
    if(NULL == fp){
        printf("can not open config file: %s\n", file);
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    char* conf_string = mem_alloc(file_size + 1);
    fseek(fp, 0, SEEK_SET);
    fread(conf_string,file_size,1,fp);

    conf_string[file_size] = '\n';

    fclose(fp);

    return conf_string;
}

void free_config_string(char* str){

    mem_free(str);
}

u_config *parse_config(char* conf_string){

    u_config* config = mem_alloc(sizeof(u_config));
    memset(config,0,sizeof(u_config));
    if(NULL == config){
        printf("out of memory while loading configuration\n");
        return NULL;
    }

    json_object* j_obj = json_tokener_parse(conf_string);
    json_object_object_foreach(j_obj, key, value){

        enum json_type type = json_object_get_type(value);
        if(0 == strcmp(CONF_IPADDRESS_KEY, key)
                && type == json_type_string){
            config->ip_address = strdup(json_object_get_string(value));
        }else if(0 == strcmp(CONF_PORT_KEY, key)
                && type == json_type_int){
            config->port = json_object_get_int(value);
        }else if(0 == strcmp(CONF_WEBROOT_KEY, key)
                && type == json_type_string){
            config->web_root = strdup(json_object_get_string(value));
        }else if(0 == strcmp(CONF_LOG_KEY, key)
                && type==json_type_string){
            config->log_file = strdup(json_object_get_string(value));
        }else if(0 == strcmp(CONF_FCGI_HOST_KEY, key)
                && type==json_type_string){
            config->fcgi_host = strdup(json_object_get_string(value));
        }else if(0 == strcmp(CONF_FCGI_PORT_KEY, key)
                 && type == json_type_int){
            config->fcgi_port = json_object_get_int(value);
        }
    }
    return config;
}

void free_config(u_config* config){

    FREE(config->ip_address, mem_free);
    FREE(config->web_root, mem_free);
    FREE(config->log_file, mem_free);
    FREE(config->fcgi_host, mem_free);
    config->ip_address = NULL;
    config->port = NULL;
    config->port = 0;
}
