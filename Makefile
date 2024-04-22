CFLAGS := -g -Wall -Werror -Wpedantic
BIN_DIR := bin
TARGET_SRC := $(wildcard *.c)
TARGET_EXE := $(TARGET_SRC:%.c=$(BIN_DIR)/%)

.PHONY: all clean

all: $(TARGET_EXE) | $(BIN_DIR)

clean:
	rm -vrf $(BIN_DIR)

$(BIN_DIR):
	mkdir -p $@

$(BIN_DIR)/ch2 : ch2.c | $(BIN_DIR)
	gcc $< -lm $(CFLAGS) -o $@

$(BIN_DIR)/ch3 : ch3.c | $(BIN_DIR)
	gcc $< -lm $(CFLAGS) -o $@

$(BIN_DIR)/ch5 : ch5.c | $(BIN_DIR)
	gcc $< -lm $(CFLAGS) -o $@

$(BIN_DIR)/ch6 : ch6.c | $(BIN_DIR)
	gcc $< -lm $(CFLAGS) -o $@

$(BIN_DIR)/ch13 : ch13.c | $(BIN_DIR)
	gcc $< -lm $(CFLAGS) -o $@

$(BIN_DIR)/% : %.c | $(BIN_DIR)
	gcc $< $(CFLAGS) -o $@

% : %.c | $(BIN_DIR)
	gcc $< $(CFLAGS) -o $(BIN_DIR)/$@
