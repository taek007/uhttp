//
// Created by hackeris on 15-6-20.
//

#ifndef UHTTP_LOG_H
#define UHTTP_LOG_H

#include <stdio.h>
#include "conf.h"

extern FILE* log_file;

void load_log_config(u_config* config);

void close_log_file();

int logoutf(const char* fmt,...);

#endif //UHTTP_LOG_H
