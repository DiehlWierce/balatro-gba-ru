CC := gcc
CFLAGS := -I../../include -I. \
          -g -O3 -std=gnu23 -Wall -Werror -Wno-format

SRC            := hand_levels_test.c ../../source/hand_levels.c
OUT            := build/hand_levels_test

$(OUT): $(SRC) | build
	$(CC) $(CFLAGS) -o $@ $^

build:
	mkdir -p build

clean:
	rm -f $(OUT)
