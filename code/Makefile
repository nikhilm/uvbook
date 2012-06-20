UV_PATH=$(shell pwd)/../libuv

examples=\
	helloworld\
	idle-basic\
	uvcat\
	uvtee\
	onchange

all: libuv
	for dir in $(examples); do cd $$dir; echo "--- `pwd`"; gcc -g -Wall -I$(UV_PATH)/include $(UV_PATH)/uv.a -o $$dir main.c; cd ..; done

libuv:
	make -C ../libuv

clean:
	for dir in $(examples); do cd $$dir; rm -f $$dir; cd ..; done

.PHONY: clean
