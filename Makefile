all:recv send

recv:recv.c
	cc -o recv recv.c

send:send.c
	cc -o send send.c
