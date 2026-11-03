CC := gcc
CFLAGS := -I../../include -I. \
          -g -O3 -std=gnu23 -Wall -Werror -Wno-format

SRC            := joker_batch6_test.c
OUT            := build/joker_batch6_test

$(OUT): $(SRC) | build
	$(CC) $(CFLAGS) -o $@ $^

build:
	mkdir -p build

clean:
	rm -f $(OUT)
