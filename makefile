all: sync

sync:	main.o RemoteStorage.o LocalStorage.o certificate.o util.o
	g++ `pkg-config --cflags --libs vmime` main.o RemoteStorage.o LocalStorage.o certificate.o util.o -o sync
main.o: main.cpp RemoteStorage.h certificate.h
	g++ -c main.cpp
RemoteStorage.o: RemoteStorage.h RemoteStorage.cpp
	g++ -c RemoteStorage.cpp
LocalStorage.o: LocalStorage.h LocalStorage.cpp
	g++ -c LocalStorage.cpp
certificate.o: certificate.cpp certificate.h
	g++ -c certificate.cpp
util.o: util.cpp util.h
	g++ -c util.cpp
clean: 
	rm -rf *.o prog
