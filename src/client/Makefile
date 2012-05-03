CC=gcc
CFLAGS=-c
LDFLAGS=-lpthread -lssl -lcrypto
SOURCES=client.c helpers.c callbacks.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=client.exe

all: clean $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS)
	make --directory=../shared/
	$(CC) -o $@ $(OBJECTS) ../shared/networking.o $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o $(EXECUTABLE)
