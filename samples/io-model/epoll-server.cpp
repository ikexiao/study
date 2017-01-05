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

//�����׽��ַ�����
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
	//ͨ��������������ö˿ںţ���ѡ��
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

	//����epoll_event�ṹ��ı���, ev����ע���¼�,�������ڻش�Ҫ������¼�
	struct epoll_event ev, events[CLIENT_NUM];
	//�������ڴ���accept��epollר�õ��ļ�������
	epfd = epoll_create(256);
	struct sockaddr_in clientaddr;
	struct sockaddr_in serveraddr;
	listenfd = socket(AF_INET, SOCK_STREAM, 0);

	//��socket����Ϊ��������ʽ
	//setnonblocking(listenfd);
	//������Ҫ������¼���ص��ļ�������
	ev.data.fd = listenfd;
	//����Ҫ������¼�����
	//ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
	ev.events = EPOLLIN;
	//ע��epoll�¼�
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
	int  sockfd;           //���ݽ������׽���
	char line[MAXLINE];    //���ݻ�����
	for ( ; ; ) 
	{
		//�ȴ�epoll�¼��ķ���
		nfds = epoll_wait(epfd, events, CLIENT_NUM, 500);
		//�����������������¼�
		for (i = 0; i < nfds; ++i)
		{
			if(events[i].data.fd == listenfd)//����¼�⵽һ��SOCKET�û����ӵ��˰󶨵�SOCKET�˿ڣ������µ�����
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
				//�������ڶ��������ļ�������
				ev.data.fd = connfd;
				//��������ע��Ķ������¼�
				ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
				//ev.events=EPOLLIN;
				//ע��ev
				epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev);
			}
			else if(events[i].events & EPOLLIN)  //������Ѿ����ӵ��û��������յ����ݣ���ô���ж���
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
				//��������д�������ļ�������
				ev.data.fd=sockfd;
				//��������ע���д�����¼�
				ev.events = EPOLLOUT | EPOLLET;
				//�޸�sockfd��Ҫ������¼�ΪEPOLLOUT
				epoll_ctl(epfd, EPOLL_CTL_MOD, sockfd, &ev);

			}
			else if(events[i].events & EPOLLOUT) // ��������ݷ���
			{
				sockfd = events[i].data.fd;
				strcpy(line, "ok!");
				write(sockfd, line, 4);
				//�������ڶ��������ļ�������
				ev.data.fd=sockfd;
				//��������ע��Ķ������¼�
				ev.events=EPOLLIN|EPOLLET;
				//�޸�sockfd��Ҫ������¼�ΪEPOLIN
				epoll_ctl(epfd, EPOLL_CTL_MOD, sockfd, &ev);
			}
		}
	}
	return 0;
}