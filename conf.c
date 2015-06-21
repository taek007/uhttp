//
// Created by hackeris on 15-6-21.
//

#include <stdio.h>
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
    if(NULL == config){
        printf("out of memory while loading configuration\n");
        return NULL;
    }

    
}
