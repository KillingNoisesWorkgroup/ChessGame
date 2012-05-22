CC=gcc
CFLAGS=-c
LDFLAGS=-lpthread -D_REENTRABLE
SOURCES=session.c memory_dump.c shutdown_server.c login_entry.c game_description.c \
		logging.c dynamic_array.c game_log.c lobby.c main.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=server.exe

all: clean $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS)
	make --directory=../shared/
	$(CC) -o $@ $(OBJECTS) ../shared/networking.o ../shared/common.o $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o *.exe

test: 
	./$(EXECUTABLE)
