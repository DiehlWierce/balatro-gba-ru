CC := gcc
CFLAGS := -I../../include -I. \
          -g -O3 -std=gnu23 -Wall -Werror -Wno-format

SRC            := card_rules_test.c
OUT            := build/card_rules_test

$(OUT): $(SRC) | build
	$(CC) $(CFLAGS) -o $@ $^

build:
	mkdir -p build

clean:
	rm -f $(OUT)
