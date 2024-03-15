CFLAGS = -Wall -Werror -Wpedantic -O2
TARGET_SRC = $(wildcard *.c)
TARGET_EXE = $(TARGET_SRC:%.c, %)

ch2 : ch2.c
	gcc $< -lm $(CFLAGS) -o $@

ch3 : ch3.c
	gcc $< -lm $(CFLAGS) -o $@

ch5 : ch5.c
	gcc $< -lm $(CFLAGS) -o $@

ch6 : ch6.c
	gcc $< -lm $(CFLAGS) -o $@

$(TARGET_EXE) : $@.c
	gcc $< $(CFLAGS) -o $@
