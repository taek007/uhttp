#include "server.h"
#include "conf.h"
#include "log.h"

int main(int argc, char *argv[]) {

    u_config* config = load_config_file("conf.json");
    load_log_config(config);
    start_http_server(config);

    release_log_file();
    free_config(config);
    return 0;
}


