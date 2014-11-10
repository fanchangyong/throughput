#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>

void err(char* str)
{
	perror(str);
	exit(1);
}

void do_connect()
{
	int fd = socket(AF_INET,SOCK_STREAM,0);
	if(fd==-1)
		err("socket");

	int flags = fcntl(fd,F_GETFD);
	fcntl(fd,F_SETFD,flags | O_NONBLOCK);

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8888);
	inet_aton("127.0.0.1",&addr.sin_addr);

	socklen_t len = sizeof(addr);

	if(connect(fd,(struct sockaddr*)&addr,len)==-1)
	{
		err("connect");
	}
}

void* thread_entry(void* p)
{
	for(;;)
	{
		do_connect();
	}
}

int main()
{
	int i;
	for(i=0;i<4;i++)
	{
		pthread_t t;
		pthread_create(&t,NULL,thread_entry,NULL);
	}
	getchar();
	return 0;
}
