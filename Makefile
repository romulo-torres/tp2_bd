CC := g++
CFLAGS := -std=c++17 -Wall -Wextra -I. -I./include
SRCDIR := src
BINDIR := bin

SRCS := $(wildcard *.cpp) $(wildcard $(SRCDIR)/*.cpp)
OBJS := $(SRCS:.cpp=.o)

BINARIES := $(BINDIR)/upload $(BINDIR)/findrec $(BINDIR)/seek1 $(BINDIR)/seek2

.PHONY: all clean docker build-image docker-build docker-run-upload docker-run-findrec docker-run-seek1 docker-run-seek2


build: all

all: prepare $(BINARIES)

prepare:
	mkdir -p $(BINDIR) $(SRCDIR)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@


COMMON_OBJS := $(SRCDIR)/bloco.o $(SRCDIR)/registro.o $(SRCDIR)/hashE.o
COMMON_OBJS := $(SRCDIR)/bloco.o $(SRCDIR)/registro.o $(SRCDIR)/hashE.o $(SRCDIR)/logger.o

$(BINDIR)/upload: $(SRCDIR)/upload.o $(COMMON_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

$(BINDIR)/findrec: $(SRCDIR)/findrec.o $(COMMON_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

$(BINDIR)/seek1: $(SRCDIR)/seek1.o $(COMMON_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

$(BINDIR)/seek2: $(SRCDIR)/seek2.o $(COMMON_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf $(BINDIR) *.o $(SRCDIR)/*.o

docker-build: all
	docker build -t tp2 .

build-image: docker-build

# Convenience targets to run each program in a container with ./data mounted
# Defaults:
#  - upload: /data/input.csv
#  - findrec: 123
#  - seek1: 123
#  - seek2: "Um Título Exato"

docker-run-upload:
	docker run --rm -v "$(PWD)/data:/data" tp2 ./bin/upload /data/input.csv

docker-run-findrec:
	docker run --rm -v "$(PWD)/data:/data" tp2 ./bin/findrec 123

docker-run-seek1:
	docker run --rm -v "$(PWD)/data:/data" tp2 ./bin/seek1 123

docker-run-seek2:
	docker run --rm -v "$(PWD)/data:/data" tp2 ./bin/seek2 "Um Título Exato"
