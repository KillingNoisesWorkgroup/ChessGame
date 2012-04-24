CC=gcc
CFLAGS=-c
LDFLAGS=
SOURCES=networking.c shared.c
OBJECTS=$(SOURCES:.c=.o)

all: $(OBJECTS)

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o

