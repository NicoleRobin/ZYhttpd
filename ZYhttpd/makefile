CC = g++

FLAG = $(DEBUGFLAG)

zyhttpd: main.cpp
	g++ -DDEBUG -o $@ $< $(FLAG) -pthread -lrt

clean:
	rm -rf *.o zyhttpd
