#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>

void usage(const char* proc)
{
	printf("Usage: %s, [udp_ip], [udp_port]\n", proc);
}

int main(int argc, char* argv[])
{
	if(argc != 3){
		usage(argv[0]);
		return 1;
	}

	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock < 0){
		perror("socket");
		return 2;
	}

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);

	if(bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
		perror("bind");
		return 3;
	}

	while(1){
		struct sockaddr_in client_addr;
		socklen_t len = sizeof(client_addr);
		
		//发送数据
		char buf[1024];
		if(recvfrom(sock, buf, 1024, 0, (struct sockaddr*)&client_addr, &len) < 0){
			perror("recvrom");
			return 4;
		}
		printf("recv is %s\n", buf);

		char buff[1024];
		printf("server # ");
		scanf("%s", buff);

		//接收数据
		if(sendto(sock, buff, 1024, 0, (struct sockaddr*)&client_addr, sizeof(client_addr)) < 0){
			perror("sendto");
			return 5;
		}

	}

	close(sock);
	return 0;
}
