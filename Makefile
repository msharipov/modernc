CC = gcc
CFLAGS := -g -Wall -Werror -Wpedantic -Wextra -fsanitize=address -fsanitize=leak -fsanitize=undefined
BIN_DIR := bin
TARGET_SRC := $(wildcard *.c)
LM_TARGET_SRC := $(wildcard ch2.c ch3.c ch5.c ch6.c ch13.c)
TARGET_EXE := $(TARGET_SRC:%.c=$(BIN_DIR)/%)
LM_TARGET_EXE := $(LM_TARGET_SRC:%.c=$(BIN_DIR)/%)

.PHONY: all clean

all: $(TARGET_EXE) | $(BIN_DIR) $(ASSETS_DIR)

clean:
	rm -vrf $(BIN_DIR)

$(BIN_DIR):
	mkdir -p $@

$(LM_TARGET_EXE) : $(LM_TARGET_SRC) | $(BIN_DIR)
	$(CC) $< -lm $(CFLAGS) -o $@

$(BIN_DIR)/% : %.c | $(BIN_DIR)
	$(CC) $< $(CFLAGS) -o $@

