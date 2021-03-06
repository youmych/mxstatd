# short help
# $@ - means the target
# $^ - means all prerequisites
# $< - means just the first prerequisite

CC=gcc
CXX=g++
AR=ar
RM=rm -f
INCLUDES=-I./src
OPTIMIZE=-O3
RELEASE_OPTS=-DNDEBUG
CFLAGS=$(OPTIMIZE) $(RELEASE_OPTS) -Wall -Wextra -Wimplicit-fallthrough=0 $(INCLUDES)
CXXFLAGS=-std=c++17 $(OPTIMIZE) $(RELEASE_OPTS) -Wall -Wextra -Wno-psabi $(INCLUDES)
LDLIBS=-lpthread -ldl
LDFLAGS=

prefix=/usr/local
PROJECT_DIR=$(shell pwd)

ifeq ($(debug), true)
    OPTIMIZE=-O0 -ggdb
    LDFLAGS+=-ggdb
    RELEASE_OPTS=
endif

EXECUTABLE=bin/mxstatd

CXXSOURCES= ./src/main.cpp \
	./src/app_config.cpp \
	./src/epoll_actor_proxy.cpp \
	./src/epoll_service.cpp \
	./src/net_utils.cpp \
	./src/actor_tcp_listener.cpp \
	./src/actor_tcp_reader.cpp \
	./src/actor_named_pipe_reader.cpp \
	./src/actor_udp_server.cpp \
	./src/event_queue.cpp \
	./src/statistics.cpp \
	./src/stat_map.cpp

OBJS=$(subst .cpp,.o,$(CXXSOURCES))

all: $(EXECUTABLE)
	@echo "### BINARIES AT: $(EXECUTABLE) ###"

$(EXECUTABLE): $(OBJS) $(C_OBJS)
	mkdir -p ./bin
	@echo "Linking $(EXECUTABLE)..."
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

./src/.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS) $(EXECUTABLE)

.PHONY: clean
