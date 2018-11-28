CFLAGS = -c -Wall
CC = gcc
LIBS =  -lm 

all: udp_server udp_client

udp_server: udp_server.o 
	${CC} udp_server.o -o udp_server

udp_server.o: udp_server.c
	${CC} ${CFLAGS} udp_server.c

udp_client: udp_client.o 
	${CC} udp_client.o -o udp_client

udp_client.o: udp_client.c
	${CC} ${CFLAGS} udp_client.c


clean:
	rm -f *.o *~