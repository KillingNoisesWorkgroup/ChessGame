all: shared server client

server:
	make --directory=src/server
	
client:
	make --directory=src/client
	
shared:
	make --directory=src/shared

clean:
	make clean --directory=src/server
	make clean --directory=src/client
	make clean --directory=src/shared
	rm -rf *exe
