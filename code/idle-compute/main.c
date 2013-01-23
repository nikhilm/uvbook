#include <stdio.h>

#include <uv.h>

uv_loop_t *loop;
uv_fs_t stdin_watcher;
uv_idle_t idler;
char buffer[1024];

void crunch_away(uv_idle_t* handle, int status) {
    // Compute extra-terrestrial life
    // fold proteins
    // computer another digit of PI
    // or similar
    fprintf(stderr, "Computing PI...\n");
    // just to avoid overwhelming your terminal emulator
    uv_idle_stop(handle);
}

void on_type(uv_fs_t *req) {
    if (stdin_watcher.result > 0) {
        buffer[stdin_watcher.result] = '\0';
        printf("Typed %s\n", buffer);
        uv_fs_read(loop, &stdin_watcher, 1, buffer, 1024, -1, on_type);
        uv_idle_start(&idler, crunch_away);
    }
    else {
        fprintf(stderr, "error opening file: %d\n", req->errorno);
    }
}

int main() {
    loop = uv_default_loop();

    uv_idle_init(loop, &idler);

    uv_fs_read(loop, &stdin_watcher, 1, buffer, 1024, -1, on_type);
    uv_idle_start(&idler, crunch_away);
    return uv_run(loop, UV_RUN_DEFAULT);
}
