CFLAGS := -g -Wall -Werror -Wpedantic
BIN_DIR := bin
TARGET_SRC := $(wildcard *.c)
TARGET_EXE := $(TARGET_SRC:%.c=%)

.PHONY: all

all: $(TARGET_EXE)

ch2 : ch2.c
	gcc $< -lm $(CFLAGS) -o $(BIN_DIR)/$@

ch3 : ch3.c
	gcc $< -lm $(CFLAGS) -o $(BIN_DIR)/$@

ch5 : ch5.c
	gcc $< -lm $(CFLAGS) -o $(BIN_DIR)/$@

ch6 : ch6.c
	gcc $< -lm $(CFLAGS) -o $(BIN_DIR)/$@

ch13 : ch13.c
	gcc $< -lm $(CFLAGS) -o $(BIN_DIR)/$@

% : %.c
	gcc $< $(CFLAGS) -o $(BIN_DIR)/$@
