.PHONY:all
all: tcp_client tcp_server

tcp_client:tcp_client.c
	gcc -o $@ $^
tcp_server:tcp_server.c
	gcc -o $@ $^

.PHONY:clean
clean:
	rm -f tcp_client tcp_server
