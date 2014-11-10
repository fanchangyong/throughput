#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <strings.h>
#include <unistd.h>
#include <pthread.h>

/*
 * 测试最大连接数
 */

int port = 8888;

void err(char* err)
{
	perror(err);
	exit(1);
}

int backlog = 10000;
int count = 0;

void* stat(void* p)
{
	for(;;)
	{
		printf("count:%d\n",count);
		count = 0;
		sleep(1);
	}
	return NULL;
}

int main(int argc,char** argv)
{
	int sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock==-1)
	{
		err("socket");
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

	printf("Listening port: %d\n",port);


	pthread_t t;
	pthread_create(&t,NULL,stat,NULL);

	for(;;)
	{
		int cli_sock;
		if((cli_sock = accept(sock,NULL,NULL))==-1)
		{
			err("accept");
		}
		else
		{
			++count;
		}
	}
}

