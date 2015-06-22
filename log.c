//
// Created by hackeris on 15-6-21.
//

#include <stdarg.h>
#include <stdlib.h>
#include "log.h"

FILE* log_file = 0;

void load_log_config(u_config* config){

    FILE* fp;
    if(NULL == config
       || NULL == config->log_file
       || NULL == (fp = fopen(config->log_file, "w+"))){

        log_file = stdout;
        return;
    }
    log_file = fp;
}

void close_log_file(){

    if(log_file != stdout){
        fclose(log_file);
    }
}

static char *make_message(const char* fmt, va_list ap){

    int n;
    int size = 100;
    char* p ,*np;

    if((p=malloc(size)) == NULL){
        return 0;
    }

    while (1){

        n = vsnprintf(p, size, fmt, ap);
        if(n < 0){
            return 0;
        }
        if(n < size){
            return p;
        }
        size = n*2;
        if((np = realloc(p, size)) == NULL){
            free(p);
        }else{
            p = np;
        }
    }
}

int logoutf(const char* fmt,...){

    va_list ap;
    int n;

    va_start(ap, fmt);

    char* message = make_message(fmt, ap);
    if(message){
        fprintf(log_file, message);
        free(message);
    }

    va_end(ap);

    fflush(log_file);

    return n;
}
