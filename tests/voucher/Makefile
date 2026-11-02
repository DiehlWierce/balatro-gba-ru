CC := gcc
# The game headers are copied to build/include and the GBA-only ones are replaced by the host
# stand-ins in shim/. The Shop drawing and object code (everything from the `// VoucherObject`
# marker on) needs sprites, pools and tile memory, so only the logic above it is compiled.
# -Wno-missing-declarations: clang rejects the `Item;` first-member inheritance of item.h.
# -Wno-unused-variable: s_used_layers is only used by the cut-off object section.
CFLAGS := -I build/include \
          -g -O3 -std=gnu23 -Wall -Werror -Wno-format -Wno-missing-declarations \
          -Wno-unused-variable

SRC            := voucher_test.c build/voucher_logic.c
OUT            := build/voucher_test

$(OUT): $(SRC) $(wildcard shim/*.h) | build
	rm -rf build/include
	mkdir -p build/include
	cp -R ../../include/. build/include/
	cp -R shim/. build/include/
	$(CC) $(CFLAGS) -o $@ $(SRC)

build/voucher_logic.c: ../../source/voucher.c | build
	awk '/^\/\/ VoucherObject$$/ { exit } { print }' $< > $@

build:
	mkdir -p build

clean:
	rm -rf build
