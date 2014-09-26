#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <strings.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/epoll.h>

/*
 * 测试最大连接数
 */

int port = 8888;

void err(char* err)
{
	perror(err);
	exit(1);
}

int backlog = 5;

int main(int argc,char** argv)
{
	int sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock==-1)
	{
		err("socket");
	}

	int epfd = epoll_create(1);
	if(epfd==-1)
	{
		err("epoll_create");
	}

	int on = 1;
	setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));

	struct sockaddr_in listen_addr;
	bzero(&listen_addr,sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_port = htons(port);
	listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	socklen_t len = sizeof(listen_addr);
	if(bind(sock,(const struct sockaddr*)&listen_addr,
			len)==-1)
	{
		err("bind");
	}

	if(listen(sock,backlog)==-1)
	{
		err("listen");
	}

	const int max_events = 1024;
	struct epoll_event ev,events[max_events];
	bzero(&ev,sizeof(struct epoll_event));
	ev.events = EPOLLIN;
	ev.data.fd = sock;

	if(epoll_ctl(epfd,EPOLL_CTL_ADD,sock,&ev)==-1)
	{
		err("epoll_ctl");
	}

	printf("Listening port: %d\n",port);

	int count = 0;

	for(;;)
	{
		int nfds = epoll_wait(epfd,&events,max_events,-1);
		if(nfds==-1)
		{
			err("epoll_awit");
		}
		
		int n=0;
		for(;n<nfds;n++)
		{
			if(events[n].data.fd == sock)
			{
				struct sockaddr_in client_addr;
				socklen_t len = sizeof(client_addr);
				if(accept(sock,(struct sockaddr*)&client_addr,&len)==-1)
				{
					err("accept");
				}
				else
				{
					char* str_addr = inet_ntoa(client_addr.sin_addr);
					//printf("Accepted a client:%s:%d[%d]\n",str_addr,client_addr.sin_port,++count);
				}
			}
			else
			{
				printf("unknown fd:%d\n",events[n].data.fd);
				break;
			}
		}
	}
}

