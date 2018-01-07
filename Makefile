CC=g++ -std=c++11
CFLAGS= -g

all: proxy.cpp
	$(CC) $(CFLAGS) proxy.cpp -o proxy -pthread

clean:
	rm -rf proxy