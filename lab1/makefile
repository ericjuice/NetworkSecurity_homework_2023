c:
	g++ ./client.cpp ./des.cpp -o client.o
s: 
	g++ ./server.cpp ./des.cpp -o server.o
all:
	g++ ./client.cpp ./des.cpp -o client.o
	g++ ./server.cpp ./des.cpp -o server.o
clean:
	rm *.o
starts:
	./server.o 127.0.0.1 8888
startc:
	./client.o 127.0.0.1 8888

des:
	g++ ./des.cpp ./deschat.cpp -o des.o