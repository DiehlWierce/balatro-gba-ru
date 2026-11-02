CC := gcc
# The game headers are copied to build/include and the GBA-only ones (tonc, sprites, the engine
# globals) are replaced by the host stand-ins in shim/, so the modules compile on the host.
# -Wno-missing-declarations: clang rejects the `Item;` first-member inheritance of consumable.h.
CFLAGS := -I build/include \
          -g -O3 -std=gnu23 -Wall -Werror -Wno-format -Wno-missing-declarations

SRC            := card_modifiers_test.c ../../source/card_modifiers.c ../../source/util.c
OUT            := build/card_modifiers_test

$(OUT): $(SRC) $(wildcard shim/*.h) | build
	rm -rf build/include
	mkdir -p build/include
	cp -R ../../include/. build/include/
	cp -R shim/. build/include/
	$(CC) $(CFLAGS) -o $@ $(SRC)

build:
	mkdir -p build

clean:
	rm -rf build
