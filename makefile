# author: Giancarlo Buenaflor <e51837398@student.tuwien.ac.at>
# date: 18.11.2020

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall $(DEFS) -g

GENERATOROBJECT = generatormain.o sharedmem.o
SUPERVISOROBJECT = supervisormain.o sharedmem.o

.PHONY: all clean
all: generator supervisor

supervisor: $(SUPERVISOROBJECT)
	$(CC) $(LDFLAGS) -o $@ $^ -lpthread -lrt
generator: $(GENERATOROBJECT) -lpthread -lrt
	$(CC) $(LDFLAGS) -o $@ $^ 
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

supervisormain.o: supervisormain.c sharedmem.h
generatormain.o: generatormain.c sharedmem.h
sharedmem.o: sharedmem.c sharedmem.h

clean:
	rm -rf *.o generator supervisor
