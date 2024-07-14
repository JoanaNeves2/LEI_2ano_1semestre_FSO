
CFLAGS= -Wall 


all: stocks  client


stocks:	stocks.c shared_board.h
	cc $(CFLAGS) -o stocks stocks.c

client:	client.c shared_board.h
	cc $(CFLAGS) -o client client.c


clean:
	rm -f stocks client
