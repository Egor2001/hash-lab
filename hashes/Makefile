CC= g++
LD = $(CC)

CFLAGS= -std=c++17 -Wall -Wextra -pedantic `pkg-config openssl --cflags`
# CFLAGS += -O3
CFLAGS += -O0 -ggdb

LFLAGS= `pkg-config openssl --libs`

SRCFILE= taskA.cpp
TARGET= test

all:
	$(CC) $(CFLAGS) $(LFLAGS) $(SRCFILE) -o $(TARGET)
