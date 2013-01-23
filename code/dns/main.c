#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

uv_loop_t *loop;

uv_buf_t alloc_buffer(uv_handle_t *handle, size_t suggested_size) {
    return uv_buf_init((char*) malloc(suggested_size), suggested_size);
}

void on_read(uv_stream_t *client, ssize_t nread, uv_buf_t buf) {
    if (nread == -1) {
        if (uv_last_error(loop).code != UV_EOF)
            fprintf(stderr, "Read error %s\n", uv_err_name(uv_last_error(loop)));
        uv_close((uv_handle_t*) client, NULL);
        free(client);
        return;
    }

    char *data = (char*) malloc(sizeof(char) * (nread+1));
    data[nread] = '\0';
    strncpy(data, buf.base, nread);

    fprintf(stderr, "%s", data);
    free(data);
    free(buf.base);
}

void on_connect(uv_connect_t *req, int status) {
    if (status == -1) {
        fprintf(stderr, "connect failed error %s\n", uv_err_name(uv_last_error(loop)));
        free(req);
        return;
    }

    uv_read_start((uv_stream_t*) req->data, alloc_buffer, on_read);
    free(req);
}

void on_resolved(uv_getaddrinfo_t *resolver, int status, struct addrinfo *res) {
    if (status == -1) {
        fprintf(stderr, "getaddrinfo callback error %s\n", uv_err_name(uv_last_error(loop)));
        return;
    }

    char addr[17] = {'\0'};
    uv_ip4_name((struct sockaddr_in*) res->ai_addr, addr, 16);
    fprintf(stderr, "%s\n", addr);

    uv_connect_t *connect_req = (uv_connect_t*) malloc(sizeof(uv_connect_t));
    uv_tcp_t *socket = (uv_tcp_t*) malloc(sizeof(uv_tcp_t));
    uv_tcp_init(loop, socket);

    connect_req->data = (void*) socket;
    uv_tcp_connect(connect_req, socket, *(struct sockaddr_in*) res->ai_addr, on_connect);

    uv_freeaddrinfo(res);
}

int main() {
    loop = uv_default_loop();

    struct addrinfo hints;
    hints.ai_family = PF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = 0;

    uv_getaddrinfo_t resolver;
    fprintf(stderr, "irc.freenode.net is... ");
    int r = uv_getaddrinfo(loop, &resolver, on_resolved, "irc.freenode.net", "6667", &hints);

    if (r) {
        fprintf(stderr, "getaddrinfo call error %s\n", uv_err_name(uv_last_error(loop)));
        return 1;
    }
    return uv_run(loop, UV_RUN_DEFAULT);
}
