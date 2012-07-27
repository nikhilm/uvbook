examples=\
	helloworld\
	idle-basic\
	uvcat\
	uvtee\
	onchange\
	thread-create\
	queue-work

UV_PATH=$(shell pwd)/../libuv
CFLAGS=-g -Wall -I$(UV_PATH)/include $(UV_PATH)/uv.a

uname_S=$(shell uname -s)

ifeq (Darwin, $(uname_S))
CFLAGS+=-framework CoreServices
endif

ifeq (Linux, $(uname_S))
CFLAGS+=-lrt
endif
all: libuv
	for dir in $(examples); do cd $$dir; echo "--- `pwd`"; gcc $(CFLAGS) -o $$dir main.c; cd ..; done

libuv:
	make -C ../libuv

clean:
	for dir in $(examples); do cd $$dir; rm -f $$dir; cd ..; done

.PHONY: clean
