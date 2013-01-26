#include <stdio.h>
#include <uv.h>

int main() {
    uv_loop_t *loop = uv_loop_new();

    printf("Now quitting.\n");
    uv_run(loop, UV_RUN_DEFAULT);

    return 0;
}
