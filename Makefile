# Compiler
CC := gcc

# Compiler flags
CFLAGS := -Wall -Wextra -std=gnu99 -g 

# Source files
SRCS := main.c core.c socket.c csapp.c

# Object files
OBJS := $(SRCS:.c=.o)

# Executable name
TARGET := webserver

# Default target
all: $(TARGET)
run: $(TARGET)
	./$(TARGET) 10339
# Compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Link object files into executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

# Clean up object files and executable
clean:
	rm -f $(OBJS) $(TARGET)