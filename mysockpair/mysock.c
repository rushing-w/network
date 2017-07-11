/*************************************************************************
	> File Name: mysock.c
	> Author: rushing
	> Mail: wangchong_2018@yeah.net 
	> Created Time: Tue 11 Jul 2017 01:53:05 AM PDT
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<string.h>

int main()
{
	int sv[2];
	if(socketpair(PF_LOCAL, SOCK_STREAM, 0, sv) < 0)
	{
		perror("socketpair");
		return 1;
	}

	pid_t id = fork();
	if(id < 0)
	{
		perror("fork");
		return 2;
	}
	else if(id == 0)//child
	{
		close(sv[0]);
		const char* msg = "i am child";
		char buf[1024];
		while(1)
		{
			write(sv[1], msg, strlen(msg));
			ssize_t s = read(sv[1], buf, sizeof(buf)-1);
			if(s < 0)
			{
				perror("read");
				return 3;
			}
			else if(s == 0)
			{
				printf("father quit\n");
				return 4;
			}
			else
			{
				buf[s] = 0;
				printf("father->child: %s\n", buf);
			}
			sleep(1);
		}
		close(sv[1]);
	}
	else//father
	{
		close(sv[1]);
		const char* msg = "i am father";
		char buf[1024];
		while(1)
		{
			ssize_t s = read(sv[0], buf, sizeof(buf)-1);
			if(s < 0)
			{
				perror("read");
				return 5;
			}
			else if(s == 0)
			{
				printf("child quit\n");
				return 6;
			}
			else
			{
				buf[s] = 0;
				printf("child->father: %s\n", buf);
			}
			write(sv[0], msg, strlen(msg));
			sleep(1);
		}
		close(sv[0]);
	}

	return 0;
}
