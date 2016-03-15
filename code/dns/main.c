#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
  buf->base = malloc(suggested_size);
  buf->len = suggested_size;
}

void on_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
    if (nread < 0) {
        if (nread != UV_EOF)
            fprintf(stderr, "Read error %s\n", uv_err_name(nread));
        uv_close((uv_handle_t*) client, NULL);
        free(client);
        return;
    }

    char *ts = buf->base, *es = buf->base + nread;
    while(ts <= es)
      putchar(*ts++);
    
    free(buf->base);
}

void on_connect(uv_connect_t *req, int status) {
    if (status < 0) {
        fprintf(stderr, "connect failed error %s\n", uv_err_name(status));
        free(req);
        return;
    }

    uv_read_start((uv_stream_t*) req->handle, alloc_buffer, on_read);
    free(req);
}

void on_resolved(uv_getaddrinfo_t *resolver, int status, struct addrinfo *res) {
    if (status < 0) {
        fprintf(stderr, "getaddrinfo callback error %s\n", uv_err_name(status));
        return;
    }

    char addr[17] = {'\0'};
    uv_ip4_name((struct sockaddr_in*) res->ai_addr, addr, 16);
    fprintf(stderr, "%s\n", addr);

    uv_connect_t *connect_req = malloc(sizeof(uv_connect_t));
    uv_tcp_t *socket = malloc(sizeof(uv_tcp_t));
    uv_tcp_init(uv_default_loop(), socket);

    uv_tcp_connect(connect_req, socket, (const struct sockaddr*) res->ai_addr, on_connect);

    uv_freeaddrinfo(res);
}

int main() {
    uv_loop_t *loop = uv_default_loop();
    struct addrinfo hints = { 0, PF_INET, SOCK_STREAM, IPPROTO_TCP };
    uv_getaddrinfo_t resolver;
    int r = uv_getaddrinfo(loop, &resolver, on_resolved, "irc.freenode.net", "6667", &hints);
    
    fprintf(stderr, "irc.freenode.net is... ");
    if (r) {
        fprintf(stderr, "getaddrinfo call error %s\n", uv_err_name(r));
        return 1;
    }
    return uv_run(loop, UV_RUN_DEFAULT);
}
