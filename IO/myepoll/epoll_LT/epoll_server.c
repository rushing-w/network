/*************************************************************************
	> File Name: epoll_server.c
	> Author: rushing
	> Mail: wangchong_2018@yeah.net 
	> Created Time: Tue 11 Jul 2017 08:19:39 PM PDT
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/epoll.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<string.h>

#define SIZE 10240

static void usage(const char* proc)
{
	printf("Usage: %s [local_ip] [local_port]\n"), proc;
}

int startup(const char* ip, int port)
{
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0)
	{
		perror("socket");
		return 2;
	}
	
	int opt = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	struct sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_port = htons(port);
	local.sin_addr.s_addr = inet_addr(ip);

	if(bind(sock, (struct sockaddr*)&local, sizeof(local)) < 0)
	{
		perror("bind");
		return 3;
	}

	if(listen(sock, 5) < 0)
	{
		perror("listen");
		return 4;
	}
	return sock;
}

int main(int argc, char* argv[])
{
	if(argc != 3)
	{
		usage(argv[0]);
		return 1;
	}

	int listen_sock = startup(argv[1], atoi(argv[2]));//获取监听套接字

	int epfd = epoll_create(256);//创建epoll模型
	if(epfd < 0)
	{
		perror("epoll_create");
		return 5;
	}
	
	printf("epfd: %d, listen_sock: %d\n", epfd, listen_sock);

	//初始化epoll事件
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = listen_sock;

	epoll_ctl(epfd, EPOLL_CTL_ADD, listen_sock, &ev);
	struct epoll_event revs[64];
	int size = sizeof(revs)/sizeof(revs[0]);
	int timeout = -1;
	while(1)
	{
		int nums = epoll_wait(epfd, revs, size, timeout);
		switch(nums)
		{
			case -1:
				perror("epoll_wait");
				break;
			case 0:
				printf("timeout...\n");
				break;
			default:
				{
					int i = 0;
					for(; i < nums; ++i)
					{
						int fd = revs[i].data.fd;
						if(fd == listen_sock && revs[i].events&EPOLLIN)//此时代表读事件就绪
						{
							struct sockaddr_in client;
							socklen_t len = sizeof(client);
							int new_sock = accept(fd, (struct sockaddr*)&client, &len);
							if(new_sock < 0)//注意：这里失败了的话不能直接退出的
							{
								perror("accept");
								continue;
							}

							printf("get a client: [%s %d]\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
							ev.events = EPOLLIN;
							ev.data.fd = new_sock;
							epoll_ctl(epfd, EPOLL_CTL_ADD, new_sock, &ev);
						}
						else if(fd != listen_sock)//先读后写
						{
							char buf[SIZE];
							if(revs[i].events&EPOLLIN)//读就绪
							{
								ssize_t s = read(fd, buf, sizeof(buf)-1);
								if(s < 0)
								{
									perror("read");
									break;
								}
								else if(s == 0)
								{
									printf("client quit\n");
									close(fd);
									epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
								}
								else
								{
									buf[s] = 0;
									printf("%s\n", buf);
									ev.events = EPOLLOUT;//写
									ev.data.fd =fd;
									epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);
								}
							}
							else if(revs[i].events&EPOLLOUT)//写就绪
							{
								const char* http_echo = "HTTP/1.0 200 OK\r\n\r\n<html><h1>hello world!</h1></html>";
								write(fd, http_echo, strlen(http_echo));
								close(fd);
								epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
							}
						}
					}
				}
				break;
		}
	}

	return 0;
}


