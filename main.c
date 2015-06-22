#include "server.h"
#include "conf.h"
#include "log.h"
#include "svrctl.h"

int main(int argc, char *argv[]) {

    u_config* config = load_config_file("/etc/uhttp/conf.json");
    load_log_config(config);
    init_srv_ctl();

    start_http_server(config);

    close_log_file();
    free_config(config);
    return 0;
}


