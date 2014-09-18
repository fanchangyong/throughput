
/*
 * Measuring Disk Read throughput
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

unsigned int max_size = 0xefffffff; // 最大文件尺寸 
int buf_size = 1024; // buf大小
char* filename = "./throughput.data";

/*
 * global variables
 */
long sum_read = 0;

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

void create_file()
{
	printf("Creating file...\n");
	int sum_written = 0;
	int fd;
	if((fd=open(filename,O_RDWR|O_CREAT|O_TRUNC,0777))==-1)
	{
		err_quit("open");
	}

	char* buf = malloc(buf_size);
	memset(buf,'a',buf_size);

	/*
	 * create file
	 */
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
		if(sum_written>=max_size)
		{
			printf("Create File Size:%d\n",max_size);
			break;
		}
	}

}

int main(int argc,char** argv)
{
	parse_args(argc,argv);

	create_file();

	pthread_t tid;
	if(pthread_create(&tid,NULL,monitor,NULL)!=0)
	{
		err_quit("pthread_create");
	}

	/*
	 * read file
	 */
	int fd = open(filename,O_RDONLY);
	if(fd==-1)
	{
		err_quit("open");
	}
	for(;;)
	{
		char *buf = malloc(buf_size);
		int readn ;
		if((readn = read(fd,buf,buf_size))==-1)
		{
			err_quit("read");
		}
		else if(readn==0)
		{
			err_quit("read");
		}
		else
		{
			sum_read += readn;
		}
	}

	return 0;
}

void* monitor(void* p)
{
	for(;;)
	{
		printf("%ld m/s\n",sum_read/1024/1024);
		sum_read = 0;
		sleep(1);
	}
}
