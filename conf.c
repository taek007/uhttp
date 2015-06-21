//
// Created by hackeris on 15-6-21.
//

#include <stdio.h>
#include <json/json.h>
#include <string.h>
#include "conf.h"
#include "mem.h"

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
            const char* address = json_object_get_string(value);
            if(config->ip_address){
                mem_free(config->ip_address);
            }
            config->ip_address = mem_alloc(strlen(address) + 1);
            if(NULL == config->ip_address){
                printf("out of memory while loading configuration\n");
                return NULL;
            }
            strcpy(config->ip_address, address);
        }else if(0 == strcmp(CONF_PORT_KEY, key)
                && type == json_type_int){
            config->port = json_object_get_int(value);
        }else if(0 == strcmp(CONF_WEBROOT_KEY, key)
                && type == json_type_string){
            const char* web_root = json_object_get_string(value);
            if(config->web_root){
                mem_free(config->web_root);
            }
            config->web_root = mem_alloc(strlen(web_root) + 1);
            if(NULL == config->web_root){
                printf("out of memory while loading configuration\n");
                return NULL;
            }
            strcpy(config->web_root, web_root);
        }
    }
    return config;
}

void free_config(u_config* config){

    mem_free(config->ip_address);
    mem_free(config->web_root);
    config->ip_address = NULL;
    config->port = NULL;
    config->port = 0;
}
