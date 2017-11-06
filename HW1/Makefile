server: server.o functionUtil.o
	g++ functionUtil.o server.o -o server -std=c++14

functionUtil.o: functionUtil.cpp
	g++ functionUtil.cpp -c -o functionUtil.o -std=c++14

server.o: server.cpp
	g++ server.cpp -c -o server.o -std=c++14

clean:
	rm -f *.o *.exe client server
