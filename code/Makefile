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
	ref-timer

UV_PATH=$(shell pwd)/../libuv
CFLAGS=-g -Wall -I$(UV_PATH)/include $(UV_PATH)/uv.a

uname_S=$(shell uname -s)

ifeq (Darwin, $(uname_S))
CFLAGS+=-framework CoreServices
endif

ifeq (Linux, $(uname_S))
CFLAGS+=-lrt
endif
all: libuv plugin-example
	for dir in $(examples); do cd $$dir; echo "--- `pwd`"; gcc $(CFLAGS) -o $$dir main.c; cd ..; done

plugin-example: plugin/hello.c
	gcc $(CFLAGS) -o plugin/plugin plugin/main.c plugin/plugin.c
	gcc -g -Wall -c -fPIC -o plugin/hello.o plugin/hello.c
	gcc -bundle -undefined dynamic_lookup -o plugin/libhello.dylib plugin/hello.o

libuv:
	make -C ../libuv

clean:
	for dir in $(examples); do cd $$dir; rm -f $$dir; cd ..; done
	rm -rf plugin/*.o plugin/libhello.dylib

.PHONY: clean
