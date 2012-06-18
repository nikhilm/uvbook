#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <uv.h>

uv_fs_t open_req;
uv_fs_t read_req;

char buffer[1024];

void on_read(uv_fs_t *req) {
    uv_fs_req_cleanup(req);
    if (req->result < 0) {
        fprintf(stderr, "Read error: %s\n", uv_strerror(uv_last_error(uv_default_loop())));
    }
    else if (req->result == 0) {
    }
    else {
        write(1, buffer, req->result);
        uv_fs_read(uv_default_loop(), &read_req, open_req.result, buffer, sizeof(buffer), req->result, on_read);
    }
}

void on_open(uv_fs_t *req) {
    if (req->result != -1) {
        uv_fs_read(uv_default_loop(), &read_req, req->result, buffer, sizeof(buffer), -1, on_read);
    }
    else {
        fprintf(stderr, "error opening file: %d\n", req->errorno);
    }
    uv_fs_req_cleanup(req);
}

int main(int argc, char **argv) {
    uv_fs_open(uv_default_loop(), &open_req, argv[1], O_RDONLY, 0, on_open);
    uv_run(uv_default_loop());
    return 0;
}
