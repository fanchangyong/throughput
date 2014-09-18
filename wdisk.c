/*
 * Measuring Disk Write throughput
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

/*
 * Options
 */

int buf_size = 1024; // 写buf大小
char* filename = "./throughput.data";

/*
 * global variables
 */
long sum_written = 0;

void err_quit(char* err)
{
	perror(err);
	exit(1);
}

void* monitor(void* p);

int parse_args(int argc,char** argv)
{
	char c;
	while((c=getopt(argc,argv,"b:"))!=-1)
	{
		switch(c)
		{
			case 'b':
				buf_size = atoi(optarg);
				break;
		}
	}
	if(optind<argc)
	{
		filename = argv[optind];
	}
	
	return 0;
}

int main(int argc,char** argv)
{
	parse_args(argc,argv);
	int fd;
	if((fd=open(filename,O_RDWR|O_CREAT|O_TRUNC,0777))==-1)
	{
		err_quit("open");
	}

	pthread_t tid;
	if(pthread_create(&tid,NULL,monitor,NULL)!=0)
	{
		err_quit("pthread_create");
	}
	char* buf = malloc(buf_size);

	memset(buf,'a',buf_size);

	for(;;)
	{
		int written ;
		if((written = write(fd,buf,buf_size))==-1)
		{
			err_quit("write");
		}
		else if(written==0)
		{
			printf("EOF\n");
			break;
		}
		else
		{
			// write ok
			sum_written += written;
		}
	}

	return 0;
}

void* monitor(void* p)
{
	for(;;)
	{
		printf("%ld m/s\n",sum_written/1024/1024);
		sum_written = 0;
		sleep(1);
	}
}
