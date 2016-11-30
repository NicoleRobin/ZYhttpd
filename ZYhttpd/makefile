CC = g++

FLAG = $(DEBUGFLAG)

zyhttpd: main.cpp
	g++ -o $@ $< $(FLAG) -pthread -lrt

clean:
	rm -rf *.o zyhttpd
