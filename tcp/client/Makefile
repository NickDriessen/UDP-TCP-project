# Makefile for UDP Client (Cross-platform: Linux/Windows with mingw32-make support)

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Werror

# Source files
CLIENT_SRC = client.c
SYSTEM_SRC = system_functions.c

# Object files
CLIENT_OBJ = client.o
SYSTEM_OBJ = system_functions.o

# OS-specific settings
ifeq ($(OS),Windows_NT)
    TARGET = client.exe
    RM = cmd /C del /Q
    LDLIBS = -lws2_32
else
    TARGET = client
    RM = rm -f
    LDLIBS =
endif

# Default rule: Build the executable
all: $(TARGET)

# Rule to link the object file to create the executable
$(TARGET): $(CLIENT_OBJ) $(SYSTEM_OBJ)
	$(CC) $(CFLAGS) $(CLIENT_OBJ) $(SYSTEM_OBJ) -o $(TARGET) $(LDLIBS)

# Rule to compile client.c to client.o
$(CLIENT_OBJ): $(CLIENT_SRC)
	$(CC) $(CFLAGS) -c $(CLIENT_SRC) -o $(CLIENT_OBJ)

# Rule to compile system_functions.c to system_functions.o
$(SYSTEM_OBJ): $(SYSTEM_SRC)
	$(CC) $(CFLAGS) -c $(SYSTEM_SRC) -o $(SYSTEM_OBJ)

# Clean rule: Remove object file and the executable
clean:
	$(RM) $(CLIENT_OBJ) $(SYSTEM_OBJ) $(TARGET)

.PHONY: all clean
