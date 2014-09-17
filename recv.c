#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <strings.h>

#define BACKLOG 5
#define PORT 8888

void err(char* err)
{
	perror(err);
	exit(1);
}

int process_conn(int cli_sock);

int main()
{
	int sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock==-1)
	{
		err("socket");
	}

	struct sockaddr_in listen_addr;
	bzero(&listen_addr,sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_port = htons(PORT);
	listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	socklen_t len = sizeof(listen_addr);
	if(bind(sock,(const struct sockaddr*)&listen_addr,
			len)==-1)
	{
		err("bind");
	}

	if(listen(sock,BACKLOG)==-1)
	{
		err("listen");
	}
	
	for(;;)
	{
		struct sockaddr_in client_addr;
		socklen_t len = sizeof(client_addr);
		int cli_sock;
		if((cli_sock = accept(sock,(struct sockaddr*)&client_addr,&len))==-1)
		{
			err("accept");
		}
		else
		{
			char* str_addr = inet_ntoa(client_addr.sin_addr);
			printf("Accepted a client:%s:%d\n",str_addr,client_addr.sin_port);
			process_conn(cli_sock);
		}
	}
}

int process_conn(int cli_sock)
{
	char buf[1024];
	for(;;)
	{
		int ret;
		if((ret=recv(cli_sock,buf,sizeof(buf),0))==-1)
		{
			err("recv");
		}
		else if(ret==0)
		{
			printf("EOF\n");
			break;
		}
		printf("Received:%s\n",buf);
	}

	return 0;
}
