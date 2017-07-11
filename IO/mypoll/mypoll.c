/*************************************************************************
	> File Name: poll_server.c
	> Author: rushing
	> Mail: wangchong_2018@yeah.net 
	> Created Time: Sun 09 Jul 2017 07:01:58 PM PDT
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<poll.h>


int main()
{
	struct pollfd fds;
	fds.fd = 0;
	fds.events = POLLIN;
	fds.revents = 0;

	int timeout = 1000;
	
	while(1)
	{
		int num = poll(&fds, 1, timeout);
		switch(num)
		{
			case -1:
				perror("poll");
				break;
			case 0:
				printf("timeout...\n");
				break;
			default:
				{
					char buf[1024];
					if(fds.fd == 0 && fds.revents&POLLIN) 
					{
						ssize_t s = read(0, buf, sizeof(buf)-1);
						if(s < 0)
						{
							perror("read");
							return 1;
						}
						buf[s-1] = 0;
						printf("echo # %s\n", buf);
					}
				}
				break;
		}
	}
}


