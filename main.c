#include "server.h"
#include "conf.h"

int main(int argc, char *argv[]) {

    u_config* config = load_config_file("conf.json");
    start_http_server(config);
    return 0;
}


