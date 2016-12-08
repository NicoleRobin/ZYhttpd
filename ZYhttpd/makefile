CC = g++

FLAG = $(DEBUGFLAG)

zyhttpd: main.cpp
	g++ -DDEBUG -o $@ $< $(FLAG) ./libzylog.a -pthread -lrt

clean:
	rm -rf *.o zyhttpd
