CC=gcc
CFLAGS=-c
LDFLAGS=
SOURCES=networking.c helpers.c common.c encryption.c
OBJECTS=$(SOURCES:.c=.o)

all: $(OBJECTS)

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o

