CC=g++
REQ_FILES=include/easysocket.cpp include/es_Client.cpp include/es_Server.cpp
REQ_LIBS=-lpthread
DEBUG_FLAGS=-g
WARN_FLAGS=-Wall -Wextra

OBJ_DIR=obj
SRC_DIR=include
OBJS = $(OBJ_DIR)/main.o


test-client: test/cpp/test_client.cpp  $(REQ_FILES)
	@$(CC) \
	test/cpp/test_client.cpp \
	$(REQ_FILES) \
	-o test/cpp/bin/test_easysocket_C \
	$(DEBUG_FLAGS) \
	-I. \
	$(REQ_LIBS) \
	$(WARN_FLAGS)

test-server: test/cpp/test_server.cpp $(REQ_FILES)
	@$(CC) \
	test/cpp/test_server.cpp \
	$(REQ_FILES) \
	-o test/cpp/bin/test_easysocket_S \
	$(DEBUG_FLAGS) \
	-I. \
	$(REQ_LIBS) \
	$(WARN_FLAGS)

test-webserve: test/cpp/test_ex.cpp $(REQ_FILES)
	@$(CC) \
	test/cpp/test_ex.cpp \
	$(REQ_FILES) \
	-o test/cpp/bin/test_easysocket_W \
	$(DEBUG_FLAGS) \
	-I. \
	$(REQ_LIBS) \
	$(WARN_FLAGS)

test-clean:
	rm test/cpp/bin/test_easysocket_S test/cpp/bin/test_easysocket_C test/cpp/bin/test_easysocket_W

