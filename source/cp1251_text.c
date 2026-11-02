/**
 * @file cp1251_text.c
 *
 * @brief Single-byte text decoding for TTE in the Russian build.
 *
 * All strings of the Russian build are compiled to CP1251 (`-fexec-charset=CP1251` in the
 * Makefile) and the font holds 224 glyphs for the codes 32..255.
 *
 * libtonc's TTE decodes every byte >= 0x80 as the start of a UTF-8 sequence. With CP1251 text
 * this corrupts some letter pairs, e.g. "Чё" (0xD7 0xB8) is read as one 2-byte UTF-8 character.
 *
 * The Makefile links a copy of libtonc's `tte_main.o` in which `utf8_decode_char()` is a weak
 * symbol, so the strong definition below replaces it in `tte_write()` and in the console hooks:
 * one byte is always one character.
 */
#include <tonc_types.h>

uint utf8_decode_char(const char* ptr, char** endptr);

uint utf8_decode_char(const char* ptr, char** endptr)
{
    const unsigned char* src = (const unsigned char*)ptr;
    uint ch = *src;

    if (endptr != NULL)
    {
        *endptr = (char*)(src + 1);
    }

    return ch;
}
