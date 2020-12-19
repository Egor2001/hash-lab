CC= g++
LD = $(CC)

CFLAGS= -std=c++17 -Wall -Wextra -pedantic
CFLAGS += -O3
# CFLAGS += -O0 -ggdb

SRCFILE= taskA.cpp
TARGET= test

all:
	$(CC) $(CFLAGS) $(SRCFILE) -o $(TARGET)
