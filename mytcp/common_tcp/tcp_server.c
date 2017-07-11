#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
#include<stdlib.h>

static void usage(const char* proc)
{
	printf("Usage: %s [local_ip] [local_port]\n", proc);
}

int startup(const char* _ip, int _port)
{
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0){
		perror("socket");
		exit(2);
	}
	int opt = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	struct sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_port = htons(_port);//htons就是将host主机转换为net网络中的，而s代表short，l则代表long
	local.sin_addr.s_addr = inet_addr(_ip);

	if(bind(sock, (struct sockaddr*)&local, sizeof(local)) < 0){
		perror("bind");
		exit(3);
	}

	if(listen(sock, 5)){//5表示监听的最大个数
		perror("listen");
		exit(4);
	}
	return sock;
}

int main(int argc, char* argv[])
{
	if(argc != 3){
		usage(argv[0]);
		exit(1);
	}

	int server_sock = startup(argv[1],atoi(argv[2]));

	while(1){
		struct sockaddr_in client_addr; 
		socklen_t len = sizeof(client_addr);
		int client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &len);
		if(client_sock < 0){
			perror("accept");
			continue;
		}
		
		printf("IP is %s, Port is %d\n", inet_ntoa(client_addr.sin_addr), htons(client_addr.sin_port));

		char buf[1024];
		while(1){
			int ret = read(client_sock, buf, sizeof(buf)-1);
			if(ret < 0){
				perror("read");
				exit(5);
			}else if(ret == 0){
				printf("client quit\n");
				close(client_sock);
				break;
			}else{
				buf[ret] = 0;
				printf("client #:%s\n", buf);
				write(client_sock, buf, strlen(buf));
			}
		}
		close(client_sock);
	}
	close(server_sock);
	return 0;
}
