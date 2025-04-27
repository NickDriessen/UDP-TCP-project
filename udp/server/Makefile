# Makefile for UDP Server

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Werror  # Add more flags as needed, e.g., -g for debugging

# Source files
SERVER_SRC = server.c
SYSTEM_SRC = system_functions.c

# Object files
SERVER_OBJ = $(SERVER_SRC:.c=.o)
SYSTEM_OBJ = $(SYSTEM_SRC:.c=.o)

# Executable name
TARGET = server

# Default rule: Build the executable
all: $(TARGET)

# Rule to link the object files to create the executable
$(TARGET): $(SERVER_OBJ) $(SYSTEM_OBJ)
	$(CC) $(SERVER_OBJ) $(SYSTEM_OBJ) -o $(TARGET)

# Rule to compile server.c to server.o
$(SERVER_OBJ): $(SERVER_SRC)
	$(CC) $(CFLAGS) -c $(SERVER_SRC) -o $(SERVER_OBJ)

# Rule to compile system_functions.c to system_functions.o
$(SYSTEM_OBJ): $(SYSTEM_SRC)
	$(CC) $(CFLAGS) -c $(SYSTEM_SRC) -o $(SYSTEM_OBJ)

# Clean rule: Remove object files and the executable
clean:
	rm -f $(SERVER_OBJ) $(SYSTEM_OBJ) $(TARGET)

.PHONY: all clean  # Declare 'all' and 'clean' as phony targets