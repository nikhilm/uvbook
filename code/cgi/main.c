#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

uv_loop_t *loop;
uv_process_t child_req;
uv_process_options_t options;

void on_exit(uv_process_t *req, int exit_status, int term_signal) {
    fprintf(stderr, "Process exited with status %d, signal %d\n", exit_status, term_signal);
    uv_close((uv_handle_t*) req->data, NULL);
    uv_close((uv_handle_t*) req, NULL);
}

void invoke_cgi_script(uv_tcp_t *client) {
    size_t size = 500;
    char path[size];
    uv_exepath(path, &size);
    strcpy(path + (strlen(path) - strlen("cgi")), "tick");

    char* args[2];
    args[0] = path;
    args[1] = NULL;

    /* ... finding the executable path and setting up arguments ... */

    options.stdio_count = 3;
    uv_stdio_container_t child_stdio[3];
    child_stdio[0].flags = UV_IGNORE;
    child_stdio[1].flags = UV_INHERIT_STREAM;
    child_stdio[1].data.stream = (uv_stream_t*) client;
    child_stdio[2].flags = UV_IGNORE;
    options.stdio = child_stdio;

    options.exit_cb = on_exit;
    options.file = args[0];
    options.args = args;

    child_req.data = (void*) client;
    if (uv_spawn(loop, &child_req, options)) {
        fprintf(stderr, "%s\n", uv_strerror(uv_last_error(loop)));
        return;
    }
}

void on_new_connection(uv_stream_t *server, int status) {
    if (status == -1) {
        // error!
        return;
    }

    uv_tcp_t *client = (uv_tcp_t*) malloc(sizeof(uv_tcp_t));
    uv_tcp_init(loop, client);
    if (uv_accept(server, (uv_stream_t*) client) == 0) {
        invoke_cgi_script(client);
    }
    else {
        uv_close((uv_handle_t*) client, NULL);
    }
}

int main() {
    loop = uv_default_loop();

    uv_tcp_t server;
    uv_tcp_init(loop, &server);

    struct sockaddr_in bind_addr = uv_ip4_addr("0.0.0.0", 7000);
    uv_tcp_bind(&server, bind_addr);
    int r = uv_listen((uv_stream_t*) &server, 128, on_new_connection);
    if (r) {
        fprintf(stderr, "Listen error %s\n", uv_err_name(uv_last_error(loop)));
        return 1;
    }
    return uv_run(loop);
}
