CFLAGS := -g -Wall -Werror -Wpedantic -Wextra -fsanitize=address -fsanitize=leak -fsanitize=undefined
BIN_DIR := bin
ASSETS_DIR := assets
TARGET_SRC := $(wildcard *.c)
TARGET_EXE := $(TARGET_SRC:%.c=$(BIN_DIR)/%)

.PHONY: all clean ch2 ch3 ch5 ch6 ch11 ch13

all: $(TARGET_EXE) | $(BIN_DIR) $(ASSETS_DIR)

clean:
	rm -vrf $(BIN_DIR)

$(BIN_DIR):
	mkdir -p $@

$(ASSETS_DIR):
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

$(BIN_DIR)/ch11 : ch11.c | $(BIN_DIR) $(ASSETS_DIR)
	gcc $< $(CFLAGS) -o $@

$(BIN_DIR)/% : %.c | $(BIN_DIR)
	gcc $< $(CFLAGS) -o $@

ch2 : ch2.c | $(BIN_DIR)
	gcc $< -lm $(CFLAGS) -o $(BIN_DIR)/$@

ch3 : ch3.c | $(BIN_DIR)
	gcc $< -lm $(CFLAGS) -o $(BIN_DIR)/$@

ch5 : ch5.c | $(BIN_DIR)
	gcc $< -lm $(CFLAGS) -o $(BIN_DIR)/$@

ch6 : ch6.c | $(BIN_DIR)
	gcc $< -lm $(CFLAGS) -o $(BIN_DIR)/$@

ch13 : ch13.c | $(BIN_DIR)
	gcc $< -lm $(CFLAGS) -o $(BIN_DIR)/$@

ch11 : ch11.c | $(BIN_DIR) $(ASSETS_DIR)
	gcc $< $(CFLAGS) -o $(BIN_DIR)/$@

% : %.c | $(BIN_DIR)
	gcc $< $(CFLAGS) -o $(BIN_DIR)/$@
