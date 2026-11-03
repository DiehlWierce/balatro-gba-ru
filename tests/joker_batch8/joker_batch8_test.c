#include <assert.h>
#include <joker_batch8.h>
#include <stdio.h>

static void test_matador(void)
{
    assert(joker_batch8_matador_money(true) == 8);
    assert(joker_batch8_matador_money(false) == 0);
}

static void test_mime(void)
{
    assert(joker_batch8_mime_passes(0) == 1); // no Mime: the held cards score once
    assert(joker_batch8_mime_passes(1) == 2);
    assert(joker_batch8_mime_passes(2) == 3);
    assert(joker_batch8_mime_passes(3) == 4);
    assert(joker_batch8_mime_passes(9) == 1 + MIME_MAX_EXTRA_PASSES); // capped
    assert(joker_batch8_mime_passes(-1) == 1);
}

int main(void)
{
    test_matador();
    test_mime();
    puts("joker_batch8: all tests passed");
    return 0;
}
