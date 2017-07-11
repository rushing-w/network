#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<string.h>
#include<stdlib.h>
#include<netinet/in.h>

void usage(const char* proc)
{
	printf("Usage: %s [local_ip] [local_port]\n", proc);
}

int main(int argc, char* argv[])
{
	if(argc != 3){
		usage(argv[0]);
		exit(1);
	}	

	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0){
		perror("socket");
		exit(2);
	}

	struct sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_port = htons(atoi(argv[2]));
	local.sin_addr.s_addr = inet_addr(argv[1]);
	if(connect(sock, (struct sockaddr*)&local, sizeof(local)) < 0){
		perror("connect");
		exit(3);
	}
	
	printf("connect success!\n");

	char buf[1024];
	while(1){
		printf("client #:");
		fflush(stdout);

		int ret = read(0, buf, sizeof(buf)-1);
		if(ret <= 0){
			perror("read");
			exit(4);
		}else{
			buf[ret-1] = 0;
			write(sock, buf, strlen(buf));
		}
	
		ret = read(sock, buf, sizeof(buf)-1);
		if(ret < 0){
			perror("read");
			exit(5);
		}else if(ret == 0){
			printf("server quit\n");
			break;
		}else{
			buf[ret] = 0;
			printf("server #:%s\n", buf);
		}
	}
	close(sock);
	return 0;
}
