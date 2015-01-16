.PHONY:clean
CC=gcc
CFLAGS=-Wall -g
BIN=miniftpd
OBJS=main.o sysutil.o session.o ftpproto.o privparent.o str.o tunable.c parseconf.o
LIBS=-lcrypt
$(BIN):$(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -f *.o $(BIN)