CC=g++

test: tests/*.cpp
	$(CC) tests/*.cpp -o tests/test -I. -Wall -Wextra -g -lpthread
	./tests/test