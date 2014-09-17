#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <strings.h>
#include <unistd.h>

#define PORT 8888
#define ADDR "127.0.0.1"

void err(char* err)
{
	perror(err);
	exit(1);
}

int main()
{
	int sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock==-1)
	{
		err("socket");
	}

	struct sockaddr_in addr;
	bzero(&addr,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.s_addr = inet_addr(ADDR);

	socklen_t len = sizeof(addr);
	if(connect(sock,(const struct sockaddr*)&addr,len)==-1)
	{
		err("connect");
	}

	printf("** Connected to %s:%d\n",ADDR,PORT);
	sleep(5);

	return 0;
}
