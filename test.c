//
// Created by hackeris on 15-6-21.
//

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "http.h"

void test_http_parser(){

    int count = 3;
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

int main() {

    test_http_parser();
    return 0;
}