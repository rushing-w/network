.PHONY:all
all:mypoll.c poll_server poll_client
mypoll:mypoll.c
	gcc -o $@ $^
poll_server:poll_server.c
	gcc -o $@ $^
poll_client:poll_client.c

.PHONY:clean
clean:
	rm -f mypoll poll_server poll_client
