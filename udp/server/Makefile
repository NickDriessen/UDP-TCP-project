# Makefile for UDP Server (Cross-platform: Linux/Windows with mingw32-make support)

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Werror

# Source files
SERVER_SRC = server.c
SYSTEM_SRC = system_functions.c

# Object files
SERVER_OBJ = server.o
SYSTEM_OBJ = system_functions.o

# OS-specific settings
ifeq ($(OS),Windows_NT)
    TARGET = server.exe
    RM = cmd /C del /Q
    LDLIBS = -lws2_32
else
    TARGET = server
    RM = rm -f
    LDLIBS =
endif

# Default rule: Build the executable
all: $(TARGET)

# Rule to link the object files to create the executable
$(TARGET): $(SERVER_OBJ) $(SYSTEM_OBJ)
	$(CC) $(CFLAGS) $(SERVER_OBJ) $(SYSTEM_OBJ) -o $(TARGET) $(LDLIBS)

# Rule to compile server.c to server.o
$(SERVER_OBJ): $(SERVER_SRC)
	$(CC) $(CFLAGS) -c $(SERVER_SRC) -o $(SERVER_OBJ)

# Rule to compile system_functions.c to system_functions.o
$(SYSTEM_OBJ): $(SYSTEM_SRC)
	$(CC) $(CFLAGS) -c $(SYSTEM_SRC) -o $(SYSTEM_OBJ)

# Clean rule: Remove object files and the executable
clean:
	$(RM) $(SERVER_OBJ) $(SYSTEM_OBJ) $(TARGET)

.PHONY: all clean