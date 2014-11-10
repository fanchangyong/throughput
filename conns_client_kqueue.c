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
#include <sys/event.h>
#include <fcntl.h>

#include <errno.h>

extern int errno;

int kq;
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
	int addr_index=0;
	for(;;)
	{
		if(!addrs[addr_index])
		{
			break;
		}
		
		char* addr = addrs[addr_index];
		//printf("addr:%s\n",addr);

		int i;
		int continue_flag = 0;

		// server sock addr
		struct sockaddr_in server_addr;
		bzero(&server_addr,sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(port);
		inet_aton(remote_addr,&server_addr.sin_addr);


		socklen_t socklen = sizeof(struct sockaddr_in);

		const int conn_count = 300;
		for(i=0;i<conn_count;i++)
		{
			int sock = socket(AF_INET,SOCK_STREAM,0);
			if(sock==-1)
			{
				err("socket");
			}

			// set sock to nonblocking
			int flags = fcntl(sock,F_GETFL,0);
			if(flags==-1)
			{
				err("fcntl");
			}
			if(fcntl(sock,F_SETFL,flags|O_NONBLOCK)==-1)
			{
				err("fcntl");
			}

			// client sock addr
			struct sockaddr_in clt_addr;
			clt_addr.sin_family = AF_INET;
			clt_addr.sin_port = htons(0);
			inet_aton(addr,&clt_addr.sin_addr);

			// bind to specify address

			if(bind(sock,(struct sockaddr*)&clt_addr,socklen)!=0)
			{
				perror("bind");
				addr_index++;
				continue_flag = 1;
				break;
			}
			// attempt to connect
			if(connect(sock,(const struct sockaddr*)&server_addr,socklen)==0)
			{
				count++;
				printf("connect [OK],%d\n",count++);
			}
			else
			{
				if(errno==EINPROGRESS)
				{
					// add to kqueue
					struct kevent changeevent,evlist;
					//printf("Added sock:%d\n",sock);
					EV_SET(&changeevent,sock,EVFILT_WRITE,EV_ADD,0,0,0);
					int nret = 0;
					if((nret=kevent(kq,&changeevent,1,&evlist,1,NULL))==-1)
					{
						err("kevent");
					}
					if(evlist.flags & EV_ERROR)
					{
						err("kevent");
					}
					if(nret<1)
					{
						perror("kevent");
					}
					//printf("nret:%d\n",nret);
				}
				else
				{
					printf("connect1 [FAIL],%s\n",strerror(errno));
					addr_index++;
					continue_flag = 1;
					break;
				}
			}
		}

		if(continue_flag)
		{
			continue;
		}

		// wait for event
		int handled_event=0;
		while(handled_event<conn_count)
		{
			int nevt = 0;
			struct kevent kevents[conn_count];
			bzero(&kevents,sizeof(kevents));
			if((nevt=kevent(kq,NULL,0,kevents,conn_count,NULL))==-1)
			{
				err("kevent");
			}
			printf("nevt:%d\n",nevt);
			int j;
			for(j=0;j<nevt;j++)
			{
				// check error
				int fd = kevents[j].ident;
				int flags = kevents[j].flags;
				if(flags & EV_ERROR)
				{
					printf("error no:%ld:%s\n",kevents[j].data,
							strerror(kevents[j].data));
					exit(1);
				}
				int opt;
				socklen_t optlen = sizeof(opt);
				if(getsockopt(fd,SOL_SOCKET,SO_ERROR,&opt,&optlen)==-1)
				{
					err("getsockopt");
				}
				if(opt)
				{
					printf("error no 2:%d:%s\n",opt,
							strerror(opt));
					exit(1);
				}

				if(connect(fd,(const struct sockaddr*)&server_addr,socklen)==0)
				{
					count++;
					printf("connect [OK],%d\n",count++);
				}

				//printf("fd:%d,data:%ld,flags:%d\n",fd,kevents[j].data,flags);

				// successfully connected
				count++;
			}
			handled_event+=nevt;
		}

		//printf("Binding to:[%s]\n",addr);

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

	// create kqueue file descriptor
	kq = kqueue();
	if(kq==-1)
	{
		err("kqueue");
	}

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

	conn(NULL);
	/*pthread_t threads[5];*/
	
	/*for(i=0;i<1;i++)*/
	/*{*/
		/*if(pthread_create(&threads[i],NULL,conn,(void*)(long)i)!=0)*/
		/*{*/
			/*err("pthread_create");*/
		/*}*/
	/*}*/
	/*pthread_create(&threads[4],NULL,stat,NULL);*/

	/*for(i=0;i<5;i++)*/
	/*{*/
		/*pthread_join(threads[i],NULL);*/
	/*}*/

	return 0;
}

