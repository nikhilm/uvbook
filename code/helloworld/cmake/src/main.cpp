#include <iostream>
#include <uv.h>

struct my_idler {
    uv_idle_t idler_;
    int64_t counter = 0;
};

class UVManager {
    typedef std::function<void(uv_idle_t*)> cb_t;
private:
    cb_t idle_cb_;
    uv_loop_t loop_;
    my_idler idler_;
    UVManager(UVManager const & );
    UVManager &operator=(UVManager const &);

public:
    UVManager() {
        uv_loop_init(&loop_);
    }

    UVManager(uv_idle_cb icb) {
        idle_cb_ = icb;
        uv_loop_init(&loop_);
        uv_idle_init(&loop_, (uv_idle_t*)&idler_);
        uv_idle_start((uv_idle_t*)&idler_, icb);
    }

    virtual ~UVManager(){
        uv_loop_close(&loop_);
    }

    void run(){
        uv_run(&loop_, UV_RUN_DEFAULT);
    }
};

int main() {
    UVManager uvm(
        [](uv_idle_t* handle) {
            my_idler* my_h = (my_idler*)handle;
            my_h->counter++;
            if (my_h->counter >= 10e6)
                uv_idle_stop(handle);
        });
    std::cout << "running the manager.." << std::endl;
    uvm.run();
    return 0;
}
