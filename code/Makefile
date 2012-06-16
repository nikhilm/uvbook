UV_PATH=$(shell pwd)/../libuv
examples=helloworld
all: libuv
	for dir in $(examples); do cd $$dir; gcc -g -Wall -I$(UV_PATH)/include $(UV_PATH)/uv.a -o $$dir main.c; cd ..; done

libuv:
	make -C ../libuv
