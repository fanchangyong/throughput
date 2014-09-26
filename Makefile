all:recv send wdisk rdisk conns_server conns_client conns_server_epoll

recv:recv.c
	cc -o recv recv.c -pthread

send:send.c
	cc -o send send.c -pthread

wdisk:wdisk.c
	cc -o wdisk wdisk.c -pthread

rdisk:rdisk.c
	cc -o rdisk rdisk.c -pthread

conns_server:conns_server.c
	cc -o conns_server conns_server.c 

conns_client:conns_client.c
	cc -o conns_client conns_client.c

conns_server_epoll:conns_server_epoll.c
	cc -o conns_server_epoll conns_server_epoll.c

.PHONY: clean

clean:
	rm -f recv send rdisk wdisk throughput.data  conns_client conns_server conns_server_epoll
