#ifndef linux
#include <stdio.h>
int main()
{
	printf("Only support Linux!\n");
	return 0;
}
#else
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

/*
 * Gobal variables
 */
int backlog = 5;
int epfd = -1;
int sock = -1;
int count = 0;

void* worker(void* p)
{
	int id = (int)p;
	printf("thread:%d\n",id);
	const int max_events = 1024;
	struct epoll_event events[max_events];
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
					++count;
					//printf("[%d] Accepted a client:%s:%d[%d]\n",id,str_addr,client_addr.sin_port,++count);
				}
			}
			else
			{
				printf("unknown fd:%d\n",events[n].data.fd);
				break;
			}
		}
	}
	return NULL;
}

void* stat(void* p)
{
	for(;;)
	{
		printf("%d connections\n",count);
		sleep(1);
	}
}

int main(int argc,char** argv)
{
	sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock==-1)
	{
		err("socket");
	}

	epfd = epoll_create(1);
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

	struct epoll_event ev;
	bzero(&ev,sizeof(struct epoll_event));
	ev.events = EPOLLIN;
	ev.data.fd = sock;

	if(epoll_ctl(epfd,EPOLL_CTL_ADD,sock,&ev)==-1)
	{
		err("epoll_ctl");
	}

	printf("Listening port: %d\n",port);

	int count = 0;

	
	pthread_t threads[5];
	int i;
	for(i=0;i<4;i++)
	{
		if(pthread_create(&threads[i],NULL,worker,(void*)i)!=0)
		{
			err("pthread_create");
		}
	}

	if(pthread_create(&threads[i],NULL,stat,NULL)!=0)
	{
		err("pthread_create");
	}

	for(i=0;i<sizeof(threads)/sizeof(pthread_t);i++)
	{
		pthread_join(threads[i],NULL);
	}

}

#endif
