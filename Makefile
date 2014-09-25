all:recv send wdisk rdisk conns_server conns_client

recv:recv.c
	cc -o recv recv.c

send:send.c
	cc -o send send.c

wdisk:wdisk.c
	cc -o wdisk wdisk.c

rdisk:rdisk.c
	cc -o rdisk rdisk.c

conns_server:conns_server.c
	cc -o conns_server conns_server.c

conns_client:conns_client.c
	cc -o conns_client conns_client.c

.PHONY: clean

clean:
	rm -f recv send rdisk wdisk throughput.data 
