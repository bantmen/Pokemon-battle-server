PORT=30103
CFLAGS= -DPORT=\$(PORT) -g -Wall
DEPS = game.h battleserver.h

all: battle
	
battle: battleserver.o game.o
	gcc $(CFLAGS) -o battle battleserver.o game.o

%.o: %.c $(DEPS)
	gcc $(CFLAGS) -c -o $@ $<
