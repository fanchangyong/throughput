#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <strings.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <string.h>

#include <errno.h>

extern int errno;

char* remote_addr = "127.0.0.1";
char *addrs[1024];
unsigned short port = 8888;
int count = 0;

int parse_args(int argc,char** argv)
{
	int c;
	while((c=getopt(argc,argv,"s:"))!=-1)
	{
		switch(c)
		{
			case 's':
				break;
		}
	}

	if(optind<argc)
	{
		remote_addr = strdup(argv[optind++]);
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

void* conn(void* p)
{
	printf("thread:%d\n",(int)p);
	int i=0;
	for(;;)
	{
		if(!addrs[i])
		{
			break;
		}
		char* addr = addrs[i];

		struct sockaddr_in server_addr;
		bzero(&server_addr,sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(port);
		inet_aton(remote_addr,&server_addr.sin_addr);

		socklen_t len = sizeof(server_addr);

		int sock = socket(AF_INET,SOCK_STREAM,0);
		struct sockaddr_in clt_addr;
		clt_addr.sin_family = AF_INET;
		clt_addr.sin_port = htons(0);
		inet_aton(addr,&clt_addr.sin_addr);

		if(bind(sock,(struct sockaddr*)&clt_addr,len)!=0)
		{
			perror("bind");
			i++;
			continue;
		}

		if(sock==-1)
		{
			err("socket");
		}
		//printf("Binding to:[%s]\n",addr);

		if(connect(sock,(const struct sockaddr*)&server_addr,len)==0)
		{
			count++;
			//printf("connect [OK],%d\n",count++);
		}
		else
		{
			printf("connect [FAIL],%s\n",strerror(errno));
			i++;
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
	bzero(&addrs,sizeof(addrs));
	parse_args(argc,argv);

	int i = 0;
	if(!addrs[0])
	{
		struct ifaddrs* ifap;
		if(getifaddrs(&ifap)!=0)
		{
			err("getifaddrs");
		}

		for(;ifap;ifap=ifap->ifa_next)
		{
			if(ifap->ifa_addr && ifap->ifa_addr->sa_family == AF_INET)
			{
				char* str_addr = 
					inet_ntoa(((struct sockaddr_in*)ifap->ifa_addr)->sin_addr);
				
				addrs[i++] = strdup(str_addr);
			}
		}
	}

	pthread_t threads[5];
	
	for(i=0;i<4;i++)
	{
		if(pthread_create(&threads[i],NULL,conn,(void*)i)!=0)
		{
			err("pthread_create");
		}
	}
	pthread_create(&threads[4],NULL,stat,NULL);

	for(i=0;i<5;i++)
	{
		pthread_join(threads[i],NULL);
	}

	return 0;
}

