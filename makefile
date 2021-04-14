CC=g++

test-client: test/cpp/test_client.cpp include/easysocket.cpp
	@$(CC) \
	test/cpp/test_client.cpp \
	include/easysocket.cpp \
	-o test/cpp/bin/test_easysocket_C \
	-g \
	-I. \

test-server: test/cpp/test_server.cpp include/easysocket.cpp
	@$(CC) \
	test/cpp/test_server.cpp \
	include/easysocket.cpp \
	-o test/cpp/bin/test_easysocket_S \
	-g \
	-I. \