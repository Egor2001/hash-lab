CC= g++
LD = $(CC)

CFLAGS= -c -std=c++17 -O3 -I../ -Wall -Wextra -pedantic
CFLAGS += `pkg-config openssl --cflags`

LFLAGS= -pthread
LFLAGS += `pkg-config openssl --libs`

SRCFILE= bench.cpp
OBJFILE= bench.o
TARGET= bench

.PHONY: clean

all: $(OBJFILE)
	$(LD) $(LFLAGS) $(OBJFILE) -o $(TARGET)

$(OBJFILE): $(SRCFILE)
	$(CC) $(CFLAGS) $(SRCFILE) -o $(OBJFILE)

clean:
	rm -f $(OBJFILE)
