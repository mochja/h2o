C=gcc
CFLAGS=-std=gnu99 -c -Wall
LDFLAGS=-pthread
SOURCES=main.c atom.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=h2o
 # -Wextra -Werror -pedantic

all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

