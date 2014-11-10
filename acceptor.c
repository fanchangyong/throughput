#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc,char** argv)
{
	int fd = socket(AF_INET,SOCK_STREAM,0);
	if(fd==-1)
	{
		perror("socket");
		return 1;
	}


	return 0;
}
