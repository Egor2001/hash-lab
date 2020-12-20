CC= g++
LD = $(CC)

CFLAGS= -std=c++17 -Wall -Wextra -pedantic
LFLAGS= 

TABLESDIR= tables/
HASHESDIR= hashes/

CFLAGS += -O3
CFLAGS += -I$(TABLESDIR) -I$(HASHESDIR)

CFLAGS += `pkg-config openssl --cflags`
LFLAGS += `pkg-config openssl --libs`

BINDIR= bin/

SRCFILE= main.cpp
TARGET= main

HASHESTEST= test/hashestest.cpp
TABLESTEST= test/tablestest.cpp

.PHONY: tags clean 

all: $(SRCFILE) $(BINDIR)
	$(CC) $(CFLAGS) $(LFLAGS) $(SRCFILE) -o $(BINDIR)/$(TARGET)

hashestest: $(HASHESTEST) $(BINDIR)
	$(CC) $(CFLAGS) $(LFLAGS) $(HASHESTEST) -o $(BINDIR)/hashestest

tablestest: $(TABLESTEST) $(BINDIR)
	$(CC) $(CFLAGS) $(LFLAGS) $(HASHESTEST) -o $(BINDIR)/hashestest

$(BINDIR):
	mkdir -p $(BINDIR)

tags:
	ctags -Rf tags $(TABLESDIR) $(HASHESDIR)

clean:
	rm -f tags
