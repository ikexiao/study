#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <iostream>
#include <errno.h>
#define MAXLINE   10
#define SERV_PROT 8000

struct StructBuf
{
	int  num;
	char str[8];
};
//ÉèÖÃÌ×½Ó×Ö·Ç×èÈû
void setnonblocking(int sock)
{
	int opts;
	opts = fcntl(sock, F_GETFL);
	if(opts<0)
	{
		perror("fcntl(sock,GETFL)");
		exit(1);
	}
	opts = opts | O_NONBLOCK;
	if(fcntl(sock, F_SETFL, opts)<0)
	{
		perror("fcntl(sock,SETFL,opts)");
		exit(1);
	}
}

int main(void)
{
	struct sockaddr_in seraddr, cliaddr;
	char buf[MAXLINE];
	int connfd;
	char ch = 'a';
	connfd = socket(AF_INET,SOCK_STREAM, 0);
	setnonblocking(connfd);
	bzero(&cliaddr,sizeof(cliaddr));
	cliaddr.sin_family      = AF_INET;
	cliaddr.sin_port        = htons(SERV_PROT);
	cliaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	connect(connfd, (struct sockaddr *)&cliaddr, sizeof(cliaddr));

	while (true)
	{
#if 0
		int i = 0;
		for (; i < MAXLINE; i++)
		{
			buf[i] = ch;
		}
		buf[i - 1] = '\n';
		ch++;
		if ('z' == ch)
		{
			ch = 'a';
		}
		write(connfd, buf, sizeof(buf));
		memset(buf, 0, sizeof(buf));
#else
		StructBuf netBuf;
		netBuf.num = 5;
		memset(netBuf.str, 0, sizeof(netBuf.str));
		strcpy(netBuf.str, "net data!");
		write(connfd, netBuf, sizeof(netBuf));
#endif
		int realNum = 0;
		if ((realNum = read(connfd, buf, MAXLINE)) < 0)
		{
			if (errno == ECONNRESET)
			{
				close(connfd);
			}
			else
				std::cout << "readline error" << std::endl;
		} 
		std::cout << "read ret:" << realNum << buf << std::endl;
		sleep(1);
	}

	close(connfd);
	return 0;
}
