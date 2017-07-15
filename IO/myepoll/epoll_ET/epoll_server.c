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
#include<fcntl.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<string.h>
#include<errno.h>

#define SIZE 10240

typedef struct epoll_buf
{
	int fd;
	char buf[SIZE];
}epoll_buff_t, *epoll_buff_p;

static void* alloc_buff(int fd)
{
	epoll_buff_p tmp = malloc(sizeof(epoll_buff_t));
	if(!tmp)
	{
		perror("malloc");
		exit(5);
	}
	tmp->fd = fd;
	return tmp;
}

static void usage(const char* proc)
{
	printf("Usage: %s [local_ip] [local_port]\n"), proc;
}

void set_fd_nonblock(int fd)//设置文件描述符为非阻塞
{
	int ret = fcntl(fd, F_GETFL);
	if(ret < 0)
	{
		perror("fcntl");
		exit(7);
	}
	if(fcntl(fd, F_SETFL, ret|O_NONBLOCK))
	{
		perror("fcntl set nonblock err");
		exit(8);
	}
}

ssize_t myread(int fd, char* buf)//ET模式下每一次都需要将数据读完
{
	ssize_t len = 0;
	ssize_t total = 0;
	while((len = read(fd, buf+total,1024)) && len == 1024 && errno == EAGAIN)//每一次都读满len，往buf+total中存
	{
		total += len;
	}
	if(len < 0 && errno != EAGAIN)
	{
		return -1;
	}
	if(len >= 0 && len < 1024)//如果最后一次没读满len退出的，那么再将len加到total上
	{
		total += len;
	}
	return total;
}

ssize_t mywrite(int fd, char* buf, int len)//ET模式下每一次都要将数据写完
{
	ssize_t pos = 0;//表示当前需要写到的位置
	ssize_t num = 0;//表示当前写的数据个数
	ssize_t n = len;//表示还需要读的数据

	while(num = write(fd, buf+pos, n) && errno == EAGAIN && num < n)//这个循环表示数据还在写并且数据不为空，读写事件还在就绪
	{
		n -= num;
		pos += num;
	}

	if(n==num)//此时刚好是写完了
	{
		pos = n;
	}

	if(num < 0 && errno != EAGAIN)
	{
		return -1;
	}
}

void myaccept(int listen_sock, int epfd)//ET模式下需要一次性接受完
{
	struct sockaddr_in client;
	socklen_t len = sizeof(client);
	int new_sock = 0;
	while((new_sock = accept(listen_sock, (struct sockaddr*)&client, &len)) > 0)
	{
		printf("get a client: [%s %d] \n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
		struct epoll_event ev;
		set_fd_nonblock(new_sock);
		ev.events = EPOLLIN|EPOLLET;
		ev.data.ptr = alloc_buff(new_sock);
		epoll_ctl(epfd, EPOLL_CTL_ADD, new_sock, &ev);
	}
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

	int listen_sock = startup(argv[1], atoi(argv[2]));

	int epfd = epoll_create(256);
	if(epfd < 0)
	{
		perror("epoll_create");
		return 6;
	}
	
	printf("epfd: %d, listen_sock: %d\n", epfd, listen_sock);

	set_fd_nonblock(listen_sock);
	struct epoll_event ev;
	ev.events = EPOLLIN|EPOLLET;
	ev.data.ptr = alloc_buff(listen_sock);

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
					//至少一个就绪
					int i = 0;
					for(; i < nums; ++i)
					{
						epoll_buff_p ptr = (epoll_buff_p)revs[i].data.ptr;
						int fd = ptr->fd;
						if(fd == listen_sock && revs[i].events&EPOLLIN)
						{
							myaccept(listen_sock, epfd);
						}
						else if(fd != listen_sock)
						{
							char* buf = ptr->buf;	
							if(revs[i].events&EPOLLIN)//读就绪
							{
								ssize_t s = myread(fd, buf);
								if(s < 0)
								{
									perror("read");
									free(ptr);
									close(fd);
									epoll_ctl(epfd, EPOLL_CTL_DEL, fd ,NULL);
									return 7;
								}
								else if(s == 0)
								{
									printf("client quit\n");
									free(ptr);
									close(fd);
									epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
								}
								else
								{
									buf[s] = 0;
									printf("%s\n",buf);
									ev.events = EPOLLOUT|EPOLLET;
									ev.data.ptr = ptr;
									epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);
								}
							}
							else if(revs[i].events&EPOLLOUT)
							{
								const char* http_echo = "HTTP/1.0 200 OK\r\n\r\n<html><h1>Hello World!</h1></html>\n";
								ssize_t s = write(fd, http_echo, strlen(http_echo));
								if(s < 0)
								{
									close(fd);
									epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
									free(ptr);
									return 8;
								}
								ev.events = EPOLLIN;
								ev.data.ptr = ptr;
								epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);
							}
						}
					}
				}
				break;
		}
	}


	return 0;
}


