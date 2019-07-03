CFLAGS += -std=c99

.PHONY: all clean test

all:
	gcc logic.c 2048.c -o 2048

test: all
	./2048 test


clean:
	rm -f 2048
