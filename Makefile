CC= g++
LD = $(CC)

CFLAGS= -std=c++17 -Wall -Wextra -pedantic
CFLAGS += -O3

LFLAGS= 

TABLESDIR= tables/
HASHESDIR= hashes/

CFLAGS += -I$(TABLESDIR) -I$(HASHESDIR)

BINDIR= bin/

SRCFILE= taskA.cpp
TARGET= test

.PHONY: tags clean 

all: $(SRCFILE) $(BINDIR)
	$(CC) $(CFLAGS) $(SRCFILE) -o $(BINDIR)/$(TARGET)

$(BINDIR):
	mkdir -p $(BINDIR)

tags:
	ctags -Rf tags $(TABLESDIR) $(HASHESDIR)

clean:
	rm -f tags
