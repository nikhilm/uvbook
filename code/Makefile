examples=\
	helloworld\
	idle-basic\
	uvcat\
	uvtee\
	onchange\
	thread-create\
	queue-work\
	progress\
	tcp-echo-server\
	dns\
	udp-dhcp\
	idle-compute\
	ref-timer\
	spawn\
	detach\
	proc-streams\
	cgi\
	pipe-echo-server

UV_PATH=$(shell pwd)/../libuv
CFLAGS=-g -Wall -I$(UV_PATH)/include $(UV_PATH)/uv.a

uname_S=$(shell uname -s)

ifeq (Darwin, $(uname_S))
CFLAGS+=-framework CoreServices
SHARED_LIB_FLAGS=-bundle -undefined dynamic_lookup -o plugin/libhello.dylib
endif

ifeq (Linux, $(uname_S))
CFLAGS+=-lrt -ldl -lm -lev -pthread
SHARED_LIB_FLAGS=-shared -Wl,-soname,libhello.so -o plugin/libhello.so
PLUGIN_EXE_FLAGS=-Wl,-export-dynamic
endif

all: libuv plugin-example proc-streams-example tick
	for dir in $(examples); do cd $$dir; echo "--- `pwd`"; gcc $(CFLAGS) -o $$dir main.c $(UV_PATH)/uv.a; cd ..; done

plugin-example: plugin/hello.c
	gcc $(CFLAGS) $(PLUGIN_EXE_FLAGS) -o plugin/plugin plugin/main.c plugin/plugin.c $(UV_PATH)/uv.a
	gcc -g -Wall -c -fPIC -o plugin/hello.o plugin/hello.c
	gcc $(SHARED_LIB_FLAGS) plugin/hello.o

proc-streams-example: proc-streams/test.c
	gcc -g -Wall -o proc-streams/test proc-streams/test.c

tick: cgi/tick.c
	gcc -g -Wall -o cgi/tick cgi/tick.c

libuv:
	make -C ../libuv

clean:
	for dir in $(examples); do cd $$dir; rm -f $$dir; cd ..; done
	rm -rf plugin/*.o plugin/libhello.*
	rm -rf plugin/plugin

.PHONY: clean
