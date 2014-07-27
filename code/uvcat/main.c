#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <uv.h>

void on_read(uv_fs_t *req);

uv_fs_t open_req;
uv_fs_t read_req;
uv_fs_t write_req;

char buffer[1024];

void on_write(uv_fs_t *req) {
    uv_fs_req_cleanup(req);
    if (req->result < 0) {
        fprintf(stderr, "Write error: %s\n", uv_strerror(uv_last_error(uv_default_loop())));
    }
    else {
        uv_fs_read(uv_default_loop(), &read_req, open_req.result, buffer, sizeof(buffer), -1, on_read);
    }
}

void on_read(uv_fs_t *req) {
    uv_fs_req_cleanup(req);
    if (req->result < 0) {
        fprintf(stderr, "Read error: %s\n", uv_strerror(uv_last_error(uv_default_loop())));
    }
    else if (req->result == 0) {
        uv_fs_t close_req;
        // synchronous
        uv_fs_close(uv_default_loop(), &close_req, open_req.result, NULL);
    }
    else {
        uv_fs_write(uv_default_loop(), &write_req, 1, buffer, req->result, -1, on_write);
    }
}

void on_open(uv_fs_t *req) {
    if (req->result != -1) {
        uv_fs_read(uv_default_loop(), &read_req, req->result,
                   buffer, sizeof(buffer), -1, on_read);
    }
    else {
        fprintf(stderr, "error opening file: %s\n", uv_strerror((int)req->result));
    }
    uv_fs_req_cleanup(req);
}

int main(int argc, char **argv) {
    uv_fs_open(uv_default_loop(), &open_req, argv[1], O_RDONLY, 0, on_open);
    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
    return 0;
}
