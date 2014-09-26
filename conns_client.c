#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <strings.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <ifaddrs.h>

extern int errno;

char *addrs[1024];
unsigned short port = 8888;
int send_buf_size = 1024;

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

	int i=0;
	while(optind<argc)
	{
		printf("optind:%d,i:%d,argc:%d\n",optind,i,argc);
		addrs[i] = strdup(argv[optind]);
		i++;
		optind++;
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
	
	int count = 0;

	i=0;
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
		inet_aton(addr,&server_addr.sin_addr);

		socklen_t len = sizeof(server_addr);

		int sock = socket(AF_INET,SOCK_STREAM,0);
		if(sock==-1)
		{
			err("socket");
		}
		printf("Connecting to:[%s:%d],",addr,port);

		if(connect(sock,(const struct sockaddr*)&server_addr,len)==0)
		{
			printf("[OK],%d\n",count++);
		}
		else
		{
			printf("[FAIL],%s\n",strerror(errno));
			i++;
		}
	}

	return 0;
}

