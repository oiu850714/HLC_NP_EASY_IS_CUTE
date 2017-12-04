CXX := g++
ARG := -std=c++14

all: sender1 sender2 sender3 receiver1 receiver2 receiver3


#executable
sender1: sender1.o UDP_socketutil.o FILE_util.o
	$(CXX) $(ARG) sender1.o UDP_socketutil.o FILE_util.o -o sender1
sender2: sender2.o UDP_socketutil.o FILE_util.o
	$(CXX) $(ARG) sender2.o UDP_socketutil.o FILE_util.o -o sender2
sender3: sender3.o UDP_socketutil.o FILE_util.o
	$(CXX) $(ARG) sender3.o UDP_socketutil.o FILE_util.o -o sender3
receiver1: receiver1.o UDP_socketutil.o FILE_util.o
	$(CXX) $(ARG) receiver1.o UDP_socketutil.o FILE_util.o -o receiver1
receiver2: receiver2.o UDP_socketutil.o FILE_util.o
	$(CXX) $(ARG) receiver2.o UDP_socketutil.o FILE_util.o -o receiver2
receiver3: receiver3.o UDP_socketutil.o FILE_util.o
	$(CXX) $(ARG) receiver3.o UDP_socketutil.o FILE_util.o -o receiver3

#utils
UDP_socketutil.o: UDP_socketutil.cpp
	$(CXX) $(ARG) UDP_socketutil.cpp -c -o UDP_socketutil.o
FILE_util.o: FILE_util.cpp
	$(CXX) $(ARG) FILE_util.cpp -c -o FILE_util.o

#senders
sender1.o: sender1.cpp
	$(CXX) $(ARG) sender1.cpp -c -o sender1.o
sender2.o: sender2.cpp
	$(CXX) $(ARG) sender2.cpp -c -o sender2.o
sender3.o: sender3.cpp
	$(CXX) $(ARG) sender3.cpp -c -o sender3.o

#receivers
receiver1.o: receiver1.cpp
	$(CXX) $(ARG) receiver1.cpp -c -o receiver1.o
receiver2.o: receiver2.cpp
	$(CXX) $(ARG) receiver2.cpp -c -o receiver2.o
receiver3.o: receiver3.cpp
	$(CXX) $(ARG) receiver3.cpp -c -o receiver3.o


clean:
	rm -f *.o *.exe sender1 sender2 sender3 receiver1 receiver2 receiver3
