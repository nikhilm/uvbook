#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

uv_loop_t *loop;

uv_buf_t alloc_buffer(uv_handle_t *handle, size_t suggested_size) {
    return uv_buf_init((char*) calloc(suggested_size, 1), suggested_size);
}

void echo_write(uv_write_t *req, int status) {
    if (status == -1) {
        fprintf(stderr, "Write error %s\n", uv_err_name(uv_last_error(loop)));
    }
    char *base = (char*) req->data;
    free(base);
    free(req);
}

void echo_read(uv_stream_t *client, ssize_t nread, uv_buf_t buf) {
    if (nread == -1) {
        if (uv_last_error(loop).code != UV_EOF)
            fprintf(stderr, "Read error %s\n", uv_err_name(uv_last_error(loop)));
        uv_close((uv_handle_t*) client, NULL);
        return;
    }

    uv_write_t *req = (uv_write_t *) malloc(sizeof(uv_write_t));
    req->data = (void*) buf.base;
    uv_write(req, client, &buf, 1, echo_write);
}

void on_new_connection(uv_stream_t *server, int status) {
    if (status == -1) {
        // error!
        return;
    }

    uv_pipe_t *client = (uv_pipe_t*) malloc(sizeof(uv_pipe_t));
    uv_pipe_init(loop, client, 0);
    if (uv_accept(server, (uv_stream_t*) client) == 0) {
        uv_read_start((uv_stream_t*) client, alloc_buffer, echo_read);
    }
    else {
        uv_close((uv_handle_t*) client, NULL);
    }
}

void remove_sock(int sig) {
    uv_fs_t req;
    uv_fs_unlink(loop, &req, "echo.sock", NULL);
    exit(0);
}

int main() {
    loop = uv_default_loop();

    uv_pipe_t server;
    uv_pipe_init(loop, &server, 0);

    signal(SIGINT, remove_sock);

    if (uv_pipe_bind(&server, "echo.sock")) {
        fprintf(stderr, "Bind error %s\n", uv_err_name(uv_last_error(loop)));
        return 1;
    }
    if (uv_listen((uv_stream_t*) &server, 128, on_new_connection)) {
        fprintf(stderr, "Listen error %s\n", uv_err_name(uv_last_error(loop)));
        return 2;
    }
    return uv_run(loop, UV_RUN_DEFAULT);
}
