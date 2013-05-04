CFLAGS = -Wall -Wextra -pedantic -O2
LFLAGS= -lm -lpthread -O2

all : main

clean : 
	rm main *.o

main.o : main.cc transform.h wave.h
	g++ $(CFLAGS) -c -o main.o main.cc

main : main.o wave.o wave.h transform.o
	g++ $(LFLAGS) -o main main.o wave.o transform.o

wave.o : wave.cc wave.h
	g++ $(CFLAGS) -c -o wave.o wave.cc

transform.o : transform.cc transform.h wave.h
	g++ $(CFLAGS) -c -o transform.o transform.cc
