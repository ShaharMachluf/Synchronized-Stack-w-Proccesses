CC=g++
STACK=stack.o
SERVER=server.o
CLIENT=client.o
FLAGS=-g

all: server client
server: $(SERVER) $(STACK) part5.o
	$(CC) $(FLAGS) -pthread -o server $(SERVER)
client: $(CLIENT) $(STACK)
	$(CC) $(FLAGS) -o client $(CLIENT)
part5.o: $(STACK) part5.cpp
	$(CC) $(FLAGS) -c part5.cpp
$(SERVER): server.cpp
	$(CC) $(FLAGS) -c server.cpp 
$(CLIENT): client.cpp
	$(CC) $(FLAGS) -c client.cpp 
$(STACK): stack.cpp
	$(CC) $(FLAGS) -c stack.cpp  

.PHONY: clean all
clean:
	rm -f *.o server client
