C=gcc
CFLAGS=-std=gnu99 -c -Wall
LDFLAGS=-pthread
SOURCES=main.c atom.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=h2o
LOGIN=xmochn00

all: $(SOURCES) $(EXECUTABLE)

zip:
	zip -l $(LOGIN).zip $(SOURCES) $(filter-out main.h, $(SOURCES:.c=.h)) Makefile
    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@
