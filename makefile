CC=g++

test: test/cpp/test.cpp include/easysocket.cpp
	$(CC) \
	test/cpp/test.cpp \
	include/easysocket.cpp \
	-o test/cpp/bin/test_easysocket \
	-g \
	-I. \