CXX := g++
ARG := -std=c++14

all: client server

client: client.o functionUtil.o
	$(CXX) $(ARG) client.o functionUtil.o -o client

server: server.o functionUtil.o
	$(CXX) $(ARG) functionUtil.o server.o -o server

functionUtil.o: functionUtil.cpp
	$(CXX) $(ARG) functionUtil.cpp -c -o functionUtil.o

server.o: server.cpp
	$(CXX) $(ARG) server.cpp -c -o server.o

client.o: client.cpp
	$(CXX) $(ARG) client.cpp -c -o client.o

clean:
	rm -f *.o *.exe client server