#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <strings.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

/*
 * global variables
 */
int sum_bytes = 0;
pthread_mutex_t lock;

/*
 * options
 */
char* addr = "127.0.0.1";
unsigned short port = 8888;
int send_buf_size = 1024;

/*
 * function declarations
 */
void* monitor(void* p);
void add_bytes(int add);
int read_bytes();
int send_msg(int sock);

int parse_args(int argc,char** argv)
{
	int c;
	while((c=getopt(argc,argv,"s:"))!=-1)
	{
		switch(c)
		{
			case 's':
				send_buf_size = atoi(optarg);
				break;
		}
	}

	printf("optind:%d,argc:%d\n",optind,argc);
	if(optind<argc)
	{
		addr = strdup(argv[optind++]);
	}
	if(optind<argc)
	{
		port = atoi(argv[optind++]);
	}

	return 0;
}

void err(char* err)
{
	perror(err);
	exit(1);
}

int main(int argc,char** argv)
{
	parse_args(argc,argv);
	int sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock==-1)
	{
		err("socket");
	}

	struct sockaddr_in server_addr;
	bzero(&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(addr);

	socklen_t len = sizeof(server_addr);
	if(connect(sock,(const struct sockaddr*)&server_addr,len)==-1)
	{
		err("connect");
	}

	printf("** Connected to %s:%d\n",addr,port);
	printf("Send Buffer Size:%d bytes\n",send_buf_size);

	pthread_t tid;
	if(pthread_create(&tid,NULL,monitor,NULL)!=0)
	{
		err("pthread_create");
	}

	send_msg(sock);

	return 0;
}

int send_msg(int sock)
{
	char* buf = malloc(send_buf_size);
	for(;;)
	{
		int written;
		if((written = write(sock,buf,send_buf_size)) ==-1)
		{
			err("write");
		}
		else if(written == 0)
		{
			printf("Server has closed the connection!\n");
		}
		else
		{
			// has successfully written
			add_bytes(written);
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
	sum_bytes += add;
	pthread_mutex_unlock(&lock);
}

int read_bytes()
{
	int bytes ;
	pthread_mutex_lock(&lock);
	bytes = sum_bytes;
	sum_bytes = 0;
	pthread_mutex_unlock(&lock);
	return bytes;
}
