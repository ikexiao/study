#include <iostream>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

using namespace std;

#define MAXLINE    1024
#define OPEN_MAX   100
#define LISTENQ    20
#define SERV_PORT  5000
#define INFTIM     1000
#define CLIENT_NUM 100

//设置套接字非阻塞
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

int main(int argc, char* argv[])
{
	int i, maxi, listenfd, connfd,epfd,nfds, portnumber;
	ssize_t n;
	socklen_t clilen;
	//通过程序参数来设置端口号（必选）
	if (2 == argc)
	{
		if( (portnumber = atoi(argv[1])) < 0 )
		{
			fprintf(stderr,"Usage:%s portnumber/a/n",argv[0]);
			return 1;
		}
	}
	else
	{
		fprintf(stderr,"Usage:%s portnumber/a/n",argv[0]);
		return 1;
	}

	//声明epoll_event结构体的变量, ev用于注册事件,数组用于回传要处理的事件
	struct epoll_event ev, events[CLIENT_NUM];
	//生成用于处理accept的epoll专用的文件描述符
	epfd = epoll_create(256);
	struct sockaddr_in clientaddr;
	struct sockaddr_in serveraddr;
	listenfd = socket(AF_INET, SOCK_STREAM, 0);

	//把socket设置为非阻塞方式
	//setnonblocking(listenfd);
	//设置与要处理的事件相关的文件描述符
	ev.data.fd = listenfd;
	//设置要处理的事件类型
	//ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
	ev.events = EPOLLIN;
	//注册epoll事件
	epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	char local_addr[16] = {0};
	strcpy(local_addr, "127.0.0.1");
	inet_aton(local_addr, &(serveraddr.sin_addr));//htons(portnumber);

	serveraddr.sin_port = htons(portnumber);
	bind(listenfd, (sockaddr *)&serveraddr, sizeof(serveraddr));
	listen(listenfd, LISTENQ);
	maxi = 0;
	int  sockfd;           //数据交互的套接字
	char line[MAXLINE];    //数据缓冲区
	for ( ; ; ) 
	{
		//等待epoll事件的发生
		nfds = epoll_wait(epfd, events, CLIENT_NUM, 500);
		//处理所发生的所有事件
		for (i = 0; i < nfds; ++i)
		{
			if(events[i].data.fd == listenfd)//如果新监测到一个SOCKET用户连接到了绑定的SOCKET端口，建立新的连接
			{
				connfd = accept(listenfd, (sockaddr *)&clientaddr, &clilen);
				if(connfd<0)
				{
					perror("connfd<0");
					exit(1);
				}
				//setnonblocking(connfd);

				char *str = inet_ntoa(clientaddr.sin_addr);
				cout << "accapt a connection from " << str << endl;
				//设置用于读操作的文件描述符
				ev.data.fd = connfd;
				//设置用于注测的读操作事件
				ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
				//ev.events=EPOLLIN;
				//注册ev
				epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev);
			}
			else if(events[i].events & EPOLLIN)  //如果是已经连接的用户，并且收到数据，那么进行读入
			{
				cout << "EPOLL get in data:" << endl;
				if ( (sockfd = events[i].data.fd) < 0)
					continue;
				if ( (n = read(sockfd, line, MAXLINE)) < 0)
				{
					if (errno == ECONNRESET) 
					{
						close(sockfd);
						events[i].data.fd = -1;
					}
					else
						std::cout<<"readline error"<<std::endl;
				} 
				else if (n == 0) 
				{
					close(sockfd);
					events[i].data.fd = -1;
				}
				line[n] = '\0';
				cout << "read content: " << line << endl;
				//设置用于写操作的文件描述符
				ev.data.fd=sockfd;
				//设置用于注测的写操作事件
				ev.events = EPOLLOUT | EPOLLET;
				//修改sockfd上要处理的事件为EPOLLOUT
				epoll_ctl(epfd, EPOLL_CTL_MOD, sockfd, &ev);

			}
			else if(events[i].events & EPOLLOUT) // 如果有数据发送
			{
				sockfd = events[i].data.fd;
				strcpy(line, "ok!");
				write(sockfd, line, 4);
				//设置用于读操作的文件描述符
				ev.data.fd=sockfd;
				//设置用于注测的读操作事件
				ev.events=EPOLLIN|EPOLLET;
				//修改sockfd上要处理的事件为EPOLIN
				epoll_ctl(epfd, EPOLL_CTL_MOD, sockfd, &ev);
			}
		}
	}
	return 0;
}