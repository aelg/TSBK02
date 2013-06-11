CFLAGS = -Wall -Wextra -pedantic -O2 -fopenmp
LFLAGS= -lm -lpthread -O2 -fopenmp

all : main coder snr

clean : 
	rm main *.o

main.o : main.cc transform.h wave.h
	g++ $(CFLAGS) -c -o main.o main.cc

main : main.o wave.o wave.h transform.o
	g++ $(LFLAGS) -o main main.o wave.o transform.o

coder.o : coder.cc transform.h wave.h
	g++ $(CFLAGS) -c -o coder.o coder.cc

coder : coder.o wave.o wave.h transform.o
	g++ $(LFLAGS) -o coder coder.o wave.o transform.o

snr.o : snr.cc wave.h
	g++ $(CFLAGS) -c -o snr.o snr.cc

snr : snr.o wave.o wave.h
	g++ $(LFLAGS) -o snr snr.o wave.o

wave.o : wave.cc wave.h
	g++ $(CFLAGS) -c -o wave.o wave.cc

transform.o : transform.cc transform.h wave.h
	g++ $(CFLAGS) -c -o transform.o transform.cc

test.o : test.cc transform.h wave.h
	g++ $(CFLAGS) -c -o test.o test.cc

test : test.o wave.o wave.h transform.o
	g++ $(LFLAGS) -o test test.o wave.o transform.o
