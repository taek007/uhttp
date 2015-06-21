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

void json_parse(){

    char * string =  "{\"sitename\" : \"joys of programming\","
            "\"tags\" : [ \"c\" , \"c++\", \"java\", \"PHP\" ],"
        "\"author-details\": { \"name\" : \"Joys of Programming\", \"Number of Posts\" : 10 } }";
    json_object *j_obj = json_tokener_parse(string);
    enum json_type type = json_object_get_type(j_obj);

    json_object_object_foreach(j_obj, key, val) {
        printf("type: ");
        type = json_object_get_type(val);
        switch (type) {
            case json_type_null:
                printf("json_type_null\n");
                break;
            case json_type_boolean:
                printf("json_type_boolean\n");
                break;
            case json_type_double:
                printf("json_type_double\n");
                break;
            case json_type_int:
                printf("json_type_int\n");
                break;
            case json_type_object:
                printf("json_type_object\n");
                break;
            case json_type_array:
                printf("json_type_array\n");
                break;
            case json_type_string:
                printf("json_type_string\n");
                break;
        }
    }
}

void test_read_conf_file() {

    char* s = load_config_string("conf.json");
    printf(s);
    free_config_string(s);
}

void test_load_configure(){

    u_config* conf = parse_config(load_config_string("conf.json"));
    if(NULL == conf){
        printf("error loading conf\n");
        return;
    }

    printf("address: %s:%d\n", conf->ip_address, conf->port);
    printf("web root: %s\n", conf->web_root);

    free_config(conf);
}

int main() {

    test_http_parser();

    json_parse();

    test_read_conf_file();

    test_load_configure();

    return 0;
}