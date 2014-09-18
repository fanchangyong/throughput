#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <strings.h>
#include <unistd.h>
#include <pthread.h>


void err(char* err)
{
	perror(err);
	exit(1);
}

/*
 * global variables
 */
int sum_recv_bytes = 0;
pthread_mutex_t lock;

/*
 * options
 */

int backlog = 5;
unsigned short port = 8888;
int recv_buf_size = 1024;

/*
 * function declarations
 */
int process_conn(int cli_sock);
void* monitor(void* p);
void add_bytes(int add);
int read_bytes();

int parse_args(int argc,char** argv)
{
	int c;
	while((c=getopt(argc,argv,"b:p:r:"))!=-1)
	{
		switch(c)
		{
			case 'b':
				backlog = atoi(optarg);
				break;
			case 'p':
				port = atoi(optarg);
				break;
			case 'r':
				recv_buf_size = atoi(optarg);
				break;
		}
	}
	return 0;
}

int main(int argc,char** argv)
{
	parse_args(argc,argv);
	int sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock==-1)
	{
		err("socket");
	}

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
	printf("Backlog: %d\n",backlog);
	printf("RecvBufferSize: %d\n",recv_buf_size);

	// Init lock
	if(pthread_mutex_init(&lock,NULL)!=0)
	{
		err("pthread_mutex_init");
	}

	pthread_t tid;
	if(pthread_create(&tid,NULL,monitor,NULL)!=0)
	{
		err("pthread_create");
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
	char *buf = malloc(recv_buf_size);
	for(;;)
	{
		int ret;
		if((ret=recv(cli_sock,buf,recv_buf_size,0))==-1)
		{
			err("recv");
		}
		else if(ret==0)
		{
			printf("EOF\n");
			break;
		}
		else
		{
			add_bytes(ret);
		}
	}

	return 0;
}

void* monitor(void* p)
{
	for(;;)
	{
		int bytes = read_bytes();
		printf("%d mb/s\n",bytes/1024/1024);
		sleep(1);
	}
	return 0;
}

void add_bytes(int add)
{
	pthread_mutex_lock(&lock);
	sum_recv_bytes += add;
	pthread_mutex_unlock(&lock);
}

int read_bytes()
{
	int bytes ;
	pthread_mutex_lock(&lock);
	bytes = sum_recv_bytes;
	sum_recv_bytes = 0;
	pthread_mutex_unlock(&lock);
	return bytes;
}
