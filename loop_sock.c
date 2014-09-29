// compare loop interface and unix socket
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

void loop_server();
void loop_client();
void do_send(int );
void do_recv(int );
void do_accept(int);
void* stat(void* p);

int port = 8888;
int bytes = 0;

void err(char* str)
{
	perror(str);
	exit(1);
}

int main(int argc,char** argv)
{
	pid_t pid;
	if((pid=fork())==0) // child
	{
		printf("child process\n");
		// use child process as client
		loop_client();
	}
	else if(pid>0) // parent
	{
		loop_server();
		waitpid(pid,NULL,0);
	}
	else
	{
		err("fork");
	}
	return 0;
}

void loop_client()
{
	int sock = socket(PF_INET,SOCK_STREAM,0);
	if(sock==-1)
	{
		err("socket");
	}

	struct sockaddr_in raddr;
	raddr.sin_family = AF_INET;
	raddr.sin_port = htons(port);
	raddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if(connect(sock,(struct sockaddr*)&raddr,sizeof(struct sockaddr_in))==-1)
	{
		err("connect");
	}
	printf("connected\n");

	pthread_t tid;
	if(pthread_create(&tid,NULL,stat,NULL)!=0)
	{
		err("pthread_create");
	}
	do_send(sock);
}

void loop_server()
{
	int sock = socket(PF_INET,SOCK_STREAM,0);	
	if(sock==-1)
	{
		err("socket");
	}

	int optval = 1;
	int optlen = sizeof(optval);
	if(setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&optval,optlen)==-1)
	{
		err("setsockopt");
	}

	struct sockaddr_in laddr;
	laddr.sin_family = AF_INET;
	laddr.sin_port = htons(port);
	laddr.sin_addr.s_addr = htonl(INADDR_ANY);

	socklen_t socklen = sizeof(struct sockaddr_in);
	if(bind(sock,(struct sockaddr*)&laddr,socklen)==-1)
	{
		err("bind");
	}

	if(listen(sock,5)==-1)
	{
		err("listen");
	}

	do_accept(sock);
}

void do_accept(int sock)
{
	for(;;)
	{
		struct sockaddr_in caddr;
		socklen_t csocklen;
		int csock;
		if((csock=accept(sock,(struct sockaddr*)&caddr,&csocklen))==-1)
		{
			err("accept");
		}
		do_recv(csock);
	}
}

void do_recv(int sock)
{
	for(;;)
	{
		char buf[1024];
		if(read(sock,buf,sizeof(buf))<=0)
		{
			break;
		}
	}
}


void do_send(int sock)
{
	printf("dosend\n");
	for(;;)
	{
		char buf[1024];
		int written = 0;
		if((written=write(sock,buf,sizeof(buf)))<=0)
		{
			err("write");
		}
		bytes += written;
	}
}

void* stat(void* p)
{
	for(;;)
	{
		printf("%d mb/s\n",bytes/1024/1024);
		bytes = 0;
		sleep(1);
	}
}
