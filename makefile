# makefile

# uncomment the version of the compiler that you are using
#clang++ is for Mac OS 
#C++ = clang++ -std=c++11
# g++ is for most Linux
C++ = g++ -std=c++11

all: dataserver client

clean:
	rm -f *.o

netreqchannel.o: netreqchannel.hpp netreqchannel.cpp
	$(C++) -c -g netreqchannel.cpp

dataserver: dataserver.cpp netreqchannel.o 
	$(C++) -g -o dataserver dataserver.cpp netreqchannel.o -lpthread

pcbuffer.o: pcbuffer.hpp pcbuffer.cpp
	$(C++) -c -g pcbuffer.cpp

semaphore.o: semaphore.hpp semaphore.cpp
	$(C++) -c -g semaphore.cpp

client: client.cpp netreqchannel.o semaphore.o pcbuffer.o
	$(C++) -g -o client client.cpp netreqchannel.o semaphore.o pcbuffer.o -lpthread
