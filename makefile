CC=g++

main: main.cpp
	$(CC) main.cpp -o tests/test -Wall -Wextra -g -lpthread