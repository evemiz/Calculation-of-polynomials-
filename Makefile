# Makefile

# Variables
CC = gcc
CFLAGS = -Wall -Wextra
TARGETS = Ex3q1 Ex3q2a Ex3q2b Ex3q3a Ex3q3b

# Phony targets (targets that are not associated with files)
.PHONY: all clean

# Default target
all: $(TARGETS)

# Compile each source file into an executable
Ex3q1: Ex3q1.c
	$(CC) $(CFLAGS) $< -o $@

Ex3q2a: Ex3q2a.c
	$(CC) $(CFLAGS) $< -o $@

Ex3q2b: Ex3q2b.c
	$(CC) $(CFLAGS) $< -o $@

Ex3q3a: Ex3q3a.c
	$(CC) $(CFLAGS) $< -o $@

Ex3q3b: Ex3q3b.c
	$(CC) $(CFLAGS) $< -o $@

# Clean the generated files
clean:
	rm -f $(TARGETS)

