# makefile

all: reshell

Shell.o: Shell.cpp 
	g++ -c -g Shell.cpp

Main.o : Main.cpp
	g++ -c -g Main.cpp

reshell: Main.o Shell.o
	g++ -o reshell Main.o Shell.o

clean:
	rm *.o
