# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g

# Target executable
TARGET = diary

# Source files
SOURCES = main.c UI.c FILE.c compression.c encryption.c

# Object files
OBJECTS = $(SOURCES:.c=.o)

# Default target
all: $(TARGET)

# Link
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET)

# Compile
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean
clean:
	rm -f $(OBJECTS) $(TARGET) diary.enc

# Rebuild
rebuild: clean all

# Run
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean rebuild run
