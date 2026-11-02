/*
 * gbarun: headless mGBA runner for scripted smoke tests.
 *
 * usage: gbarun <rom.gba> <script.txt> <outdir>
 *
 * Script commands, one per line ('#' starts a comment line):
 *   wait N              run N frames
 *   press KEY [N]       hold KEY for N frames (default 3), then release it for 6 frames
 *   hold KEY            keep KEY pressed until "release KEY"
 *   release KEY
 *   shot NAME           save the screen to <outdir>/NAME.png
 *   poke8 ADDR VAL      write a byte
 *   poke32 ADDR VAL     write a 32-bit word
 *   pokestr ADDR STR    write a NUL-terminated string
 *   peek32 ADDR         print a 32-bit word
 * KEY: A B SELECT START RIGHT LEFT UP DOWN R L
 * ADDR/VAL accept decimal or 0x hex. run.py replaces {sym:NAME} and {gv:FIELD} with addresses.
 *
 * Build (Ubuntu: apt install libmgba-dev libpng-dev):
 *   cc -O2 -o gbarun gbarun.c -lmgba -lpng
 */
#include <mgba/core/core.h>
#include <mgba/core/log.h>
#include <png.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void quiet_log(struct mLogger* logger, int category, enum mLogLevel level,
                      const char* format, va_list args)
{
    (void)logger;
    (void)category;
    (void)level;
    (void)format;
    (void)args;
}

static struct mLogger s_logger = {.log = quiet_log};

static const char* KEY_NAMES[] = {"A",     "B",  "SELECT", "START", "RIGHT",
                                  "LEFT",  "UP", "DOWN",   "R",     "L"};

static int key_bit(const char* name)
{
    for (int i = 0; i < 10; i++)
    {
        if (strcmp(name, KEY_NAMES[i]) == 0)
            return i;
    }
    fprintf(stderr, "unknown key '%s'\n", name);
    exit(2);
}

static void save_png(const char* path, const color_t* buf, unsigned w, unsigned h)
{
    FILE* f = fopen(path, "wb");
    if (f == NULL)
    {
        perror(path);
        exit(1);
    }
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info = png_create_info_struct(png);
    png_init_io(png, f);
    png_set_IHDR(png, info, w, h, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);
    unsigned char* row = malloc(w * 3);
    for (unsigned y = 0; y < h; y++)
    {
        for (unsigned x = 0; x < w; x++)
        {
            uint32_t c = buf[y * w + x];
            row[x * 3 + 0] = c & 0xFF;
            row[x * 3 + 1] = (c >> 8) & 0xFF;
            row[x * 3 + 2] = (c >> 16) & 0xFF;
        }
        png_write_row(png, row);
    }
    png_write_end(png, NULL);
    png_destroy_write_struct(&png, &info);
    free(row);
    fclose(f);
}

static void run_frames(struct mCore* core, int n, unsigned long* frame)
{
    for (int i = 0; i < n; i++, (*frame)++)
        core->runFrame(core);
}

int main(int argc, char** argv)
{
    if (argc < 4)
    {
        fprintf(stderr, "usage: gbarun <rom.gba> <script.txt> <outdir>\n");
        return 2;
    }

    mLogSetDefaultLogger(&s_logger);
    struct mCore* core = mCoreFind(argv[1]);
    if (core == NULL || !core->init(core))
    {
        fprintf(stderr, "cannot create an mGBA core for %s\n", argv[1]);
        return 1;
    }

    unsigned w, h;
    core->desiredVideoDimensions(core, &w, &h);
    color_t* video = calloc(w * h, sizeof(color_t));
    core->setVideoBuffer(core, video, w);
    core->setAudioBufferSize(core, 2048);
    mCoreInitConfig(core, NULL);
    if (!mCoreLoadFile(core, argv[1]))
    {
        fprintf(stderr, "cannot load %s\n", argv[1]);
        return 1;
    }
    core->reset(core);

    FILE* script = fopen(argv[2], "r");
    if (script == NULL)
    {
        perror(argv[2]);
        return 1;
    }

    char line[256];
    uint32_t held = 0;
    unsigned long frame = 0;
    while (fgets(line, sizeof line, script))
    {
        char cmd[32] = {0}, arg1[128] = {0}, arg2[128] = {0};
        if (line[0] == '#' || sscanf(line, "%31s", cmd) != 1)
            continue;
        int nargs = sscanf(line, "%31s %127s %127s", cmd, arg1, arg2) - 1;

        if (strcmp(cmd, "wait") == 0)
        {
            run_frames(core, atoi(arg1), &frame);
        }
        else if (strcmp(cmd, "press") == 0)
        {
            uint32_t key = 1u << key_bit(arg1);
            core->setKeys(core, held | key);
            run_frames(core, nargs >= 2 ? atoi(arg2) : 3, &frame);
            core->setKeys(core, held);
            run_frames(core, 6, &frame);
        }
        else if (strcmp(cmd, "hold") == 0)
        {
            held |= 1u << key_bit(arg1);
            core->setKeys(core, held);
        }
        else if (strcmp(cmd, "release") == 0)
        {
            held &= ~(1u << key_bit(arg1));
            core->setKeys(core, held);
        }
        else if (strcmp(cmd, "poke8") == 0)
        {
            core->busWrite8(core, strtoul(arg1, NULL, 0), (uint8_t)strtoul(arg2, NULL, 0));
        }
        else if (strcmp(cmd, "poke32") == 0)
        {
            core->busWrite32(core, strtoul(arg1, NULL, 0), strtoul(arg2, NULL, 0));
        }
        else if (strcmp(cmd, "pokestr") == 0)
        {
            uint32_t addr = strtoul(arg1, NULL, 0);
            for (size_t i = 0; i <= strlen(arg2); i++)
                core->busWrite8(core, addr + i, (uint8_t)arg2[i]);
        }
        else if (strcmp(cmd, "peek32") == 0)
        {
            uint32_t addr = strtoul(arg1, NULL, 0);
            printf("frame %lu: [%s] = %u\n", frame, arg1, core->busRead32(core, addr));
        }
        else if (strcmp(cmd, "shot") == 0)
        {
            char path[512];
            snprintf(path, sizeof path, "%s/%s.png", argv[3], arg1);
            save_png(path, video, w, h);
            printf("frame %lu: %s\n", frame, path);
        }
        else
        {
            fprintf(stderr, "unknown command '%s'\n", cmd);
            return 2;
        }
    }

    fclose(script);
    core->deinit(core);
    free(video);
    return 0;
}
