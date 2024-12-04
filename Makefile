CC = gcc
CFLAGS := -g -Wall -Werror -Wpedantic -Wextra -fsanitize=address -fsanitize=leak -fsanitize=undefined
BIN_DIR := bin
TARGET_SRC := $(wildcard *.c)
LM_TARGET_SRC := $(wildcard ch2.c ch3.c ch5.c ch6.c ch13.c)
TARGET_EXE := $(TARGET_SRC:%.c=$(BIN_DIR)/%)
LM_TARGET_EXE := $(LM_TARGET_SRC:%.c=$(BIN_DIR)/%)

.PHONY: all clean ch2 ch3 ch5 ch6 ch11 ch13

all: $(TARGET_EXE) | $(BIN_DIR) $(ASSETS_DIR)

clean:
	rm -vrf $(BIN_DIR)

$(BIN_DIR):
	mkdir -p $@

$(LM_TARGET_EXE) : $(LM_TARGET_SRC) | $(BIN_DIR)
	$(CC) $< -lm $(CFLAGS) -o $@

$(BIN_DIR)/% : %.c | $(BIN_DIR)
	$(CC) $< $(CFLAGS) -o $@

ch2 : ch2.c | $(BIN_DIR)
	$(CC) $< -lm $(CFLAGS) -o $(BIN_DIR)/$@

ch3 : ch3.c | $(BIN_DIR)
	$(CC) $< -lm $(CFLAGS) -o $(BIN_DIR)/$@

ch5 : ch5.c | $(BIN_DIR)
	$(CC) $< -lm $(CFLAGS) -o $(BIN_DIR)/$@

ch6 : ch6.c | $(BIN_DIR)
	$(CC) $< -lm $(CFLAGS) -o $(BIN_DIR)/$@

ch13 : ch13.c | $(BIN_DIR)
	$(CC) $< -lm $(CFLAGS) -o $(BIN_DIR)/$@


% : %.c | $(BIN_DIR)
	$(CC) $< $(CFLAGS) -o $(BIN_DIR)/$@
