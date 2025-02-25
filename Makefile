# Makefile
CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99
SRC = src/mpls_cli.c src/mpls_core.c src/mpls_routes.c
OBJ = $(SRC:.c=.o)
TARGET = mpls-cli

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)  

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean

