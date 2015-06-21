//
// Created by hackeris on 15-6-21.
//

#include <signal.h>

#include "server.h"
#include "svrctl.h"


void sig_int(int signo){

    stop_http_server();
}

void init_srv_ctl(){

    signal(SIGINT,sig_int);
    signal(SIGTERM, sig_int);
}

