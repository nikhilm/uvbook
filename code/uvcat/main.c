#include <assert.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <uv.h>

void on_read(uv_fs_t *req);

uv_fs_t open_req;
uv_fs_t read_req;
uv_fs_t write_req;

char slab[1024];
uv_buf_t buffer;

void on_write(uv_fs_t *req) {
    if (req->result < 0) {
        fprintf(stderr, "Write error: %s\n", uv_strerror(req->result));
    }
    else {
        buffer = uv_buf_init(slab, sizeof(slab));
        uv_fs_read(uv_default_loop(), &read_req, open_req.result, &buffer, 1, -1, on_read);
    }
}

void on_read(uv_fs_t *req) {
    if (req->result < 0) {
        fprintf(stderr, "Read error: %s\n", uv_strerror(req->result));
    }
    else if (req->result == 0) {
        uv_fs_t close_req;
        // synchronous
        uv_fs_close(uv_default_loop(), &close_req, open_req.result, NULL);
    }
    else {
        buffer.len = req->result;
        uv_fs_write(uv_default_loop(), &write_req, 1 /* stdout */, &buffer, 1, -1, on_write);
    }
}

void on_open(uv_fs_t *req) {
    // You could use a single method to deal with various filesystem calls
    // based the the type.
    assert(req->fs_type == UV_FS_OPEN);

    if (req->result > 0) {
        buffer = uv_buf_init(slab, sizeof(slab));
        uv_fs_read(uv_default_loop(), &read_req, req->result, &buffer, 1, -1, on_read);
    }
    else {
        fprintf(stderr, "error opening file: %s\n", uv_strerror(req->result));
    }
}

int main(int argc, char **argv) {
    uv_fs_open(uv_default_loop(), &open_req, argv[1], O_RDONLY, 0, on_open);
    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
    uv_fs_req_cleanup(&open_req);
    uv_fs_req_cleanup(&read_req);
    uv_fs_req_cleanup(&write_req);
    return 0;
}
