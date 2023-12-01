all: chat.c
		gcc -g -o chat chat.c; clear;

clean:
		rm chat