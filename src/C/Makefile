CC      = gcc
CFLAGS  = -Wall -Werror -I ./inc

default: getmime simple client

.PHONY: clean

getmime: getmime.c helper.c
simple: simple.c http_handler.c helper.c
client: client.c

debug: CFLAGS = -g -Wall -Werror -DDEBUG -DLOG_LEVEL=2 -I ./inc
debug: getmime server client

clean:
	rm -f *.o getmime simple client
