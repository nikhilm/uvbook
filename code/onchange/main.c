#include <stdio.h>
#include <stdlib.h>

#include <uv.h>

uv_loop_t *loop;
const char *command;

void run_command(uv_fs_event_t *handle, const char *filename, int events, int status) {
    fprintf(stderr, "Change detected in %s: ", filename);
    if (events == UV_RENAME)
        fprintf(stderr, "renamed");
    if (events == UV_CHANGE)
        fprintf(stderr, "changed");

    fprintf(stderr, " %s\n", filename ? filename : "");
    system(command);
}

int main(int argc, char **argv) {
    if (argc <= 2) {
        fprintf(stderr, "Usage: %s <command> <file1> [file2 ...]\n", argv[0]);
        return 1;
    }

    loop = uv_default_loop();
    command = argv[1];

    while (argc-- > 2) {
        fprintf(stderr, "Adding watch on %s\n", argv[argc]);
        uv_fs_event_t *fs_watcher = malloc(sizeof(*fs_watcher));
        uv_fs_event_init(loop, fs_watcher);
        uv_fs_event_start(fs_watcher, run_command, argv[argc], 0);
    }

    return uv_run(loop, UV_RUN_DEFAULT);
}
