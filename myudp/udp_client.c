#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>


void usage(char* proc)
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
	
	while(1){	
		char buff[1024];
		printf("client # ");
		scanf("%s", buff);
		if(sendto(sock, buff, 1024, 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
			perror("sendto");
			return 3;
		}

		char buf[1024];
		socklen_t len = sizeof(server_addr);
		if(recvfrom(sock, buf, 1024, 0, (struct sockaddr*)&server_addr, &len) < 0){
			perror("recvfrom");
			return 4;
		}
		printf("recv is %s\n", buf);
	}

	close(sock);
	return 0;
}
