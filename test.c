//
// Created by hackeris on 15-6-21.
//

#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <json/json.h>
#include "http.h"
#include "conf.h"

void test_http_parser(){

    int count = 3;
    extern FILE* log_file;
    log_file = stdout;
    char* test_case[] = {"case/case00.txt","case/case01.txt","case/case02.txt"};
    int i;
    http_request request;
    for(i=0;i<count; i++){

        memset(&request,0,sizeof(request));
        int fd = open(test_case[i], O_RDONLY);
        wrap_http_request(fd, &request);
        free_http_request(&request);
        close(fd);
    }
}

void test_load_configure(){

    u_config* conf = parse_config(load_config_string("conf.json"));
    if(NULL == conf){
        printf("error loading conf\n");
        return;
    }

    printf("address: %s:%d\n", conf->ip_address, conf->port);
    printf("web root: %s\n", conf->web_root);
    printf("log file: %s\n", conf->log_file);

    free_config(conf);
}

int main() {

    printf("\ntest of http parser (data from case directory).\n");
    test_http_parser();

    printf("\ntest of json config parser.\n");
    test_load_configure();

    return 0;
}