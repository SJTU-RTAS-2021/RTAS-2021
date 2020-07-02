TARGET = tcp_lat_client tcp_lat_server
OBJ = tcp_lat_client.o tcp_lat_server.o
CFLAGS = -O -Wall

all: $(TARGET)
tcp_lat_client: tcp_lat_client.c
	gcc $(CFLAGS) -o tcp_lat_client tcp_lat_client.c
tcp_lat_server: tcp_lat_server.c
	gcc $(CFLAGS) -o tcp_lat_server tcp_lat_server.c
