#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <uv.h>

uv_loop_t *loop;
uv_pipe_t queue;

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
    buf.len = nread;
    uv_write(req, client, &buf, 1, echo_write);
}

void on_new_connection(uv_pipe_t *q, ssize_t nread, uv_buf_t buf, uv_handle_type pending) {
    if (pending == UV_UNKNOWN_HANDLE) {
        // error!
        return;
    }

    uv_tcp_t *client = (uv_tcp_t*) malloc(sizeof(uv_tcp_t));
    uv_tcp_init(loop, client, 0);
    if (uv_accept((uv_stream_t*) q, (uv_stream_t*) client) == 0) {
        fprintf(stderr, "Worker %d: Accepted fd %d\n", getpid(), client->io_watcher.fd);
        uv_read_start((uv_stream_t*) client, alloc_buffer, echo_read);
    }
    else {
        uv_close((uv_handle_t*) client, NULL);
    }
}

int main() {
    loop = uv_default_loop();

    uv_pipe_init(loop, &queue, 1);
    uv_pipe_open(&queue, 0);
    uv_read2_start((uv_stream_t*)&queue, alloc_buffer, on_new_connection);
    return uv_run(loop, UV_RUN_DEFAULT);
}
