#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <strings.h>
#include <ctype.h>

char * host_name = "10.10.61.151";
int port = 8000;

int main(int argc , char * argv[])
{
	char buf[8192];
	//char message[256];
	int socket_descriptor;
	struct sockaddr_in pin;
	char * str ="A default test string";
	if(argc < 2)
	{
			printf("we will send a default test string.\n");

	}
	else
	{
			str = argv[1];
			if(argc == 3)
			{
					host_name = argv[2];
			}
	}

	bzero(&pin,sizeof(pin));
	pin.sin_family = AF_INET;
	inet_pton(AF_INET,host_name,&pin.sin_addr);
	pin.sin_port = htons(port);
	if((socket_descriptor =  socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
			perror("error opening socket \n");
			exit(1);
	}
	if(connect(socket_descriptor,(struct sockaddr * )&pin,sizeof(pin)) == -1)
	{
			perror("error connecting to socket \n");
			exit(1);
	}
	
	while (1)
	{
		printf("******************************\r\n");
		printf("sending message [%s] to server.\r\n",str);
		if( write(socket_descriptor,str,strlen(str)+1) == -1 )
		{
				perror("error in send.\r\n");
				exit(1);
		}
		printf("wait for response.\r\n");
		if( read(socket_descriptor,buf,8192) == -1 )
		{
			perror("error in receiving response from server.\r\n");
			exit(1);
		}
		printf("Response from server:[%s]\r\n",buf);
		printf("******************************\r\n");
		sleep(3);
	}
	
	close(socket_descriptor);
	return 1;
}
