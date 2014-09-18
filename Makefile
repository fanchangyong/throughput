all:recv send wdisk rdisk

recv:recv.c
	cc -o recv recv.c

send:send.c
	cc -o send send.c

wdisk:wdisk.c
	cc -o wdisk wdisk.c

rdisk:rdisk.c
	cc -o rdisk rdisk.c

.PHONY: clean

clean:
	rm -f recv send wdisk
