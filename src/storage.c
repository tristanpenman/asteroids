#include <stdio.h>

#include "debug.h"
#include "storage.h"

void storage_init()
{
    // nothing to do
}

bool storage_available()
{
#ifdef __EMSCRIPTEN__
    return false;
#else
    return true;
#endif
}

int storage_read(const char *filename, char *buffer, int read_size)
{
#ifdef __EMSCRIPTEN__
    return -1;
#endif

    FILE *f;
    size_t sz;

    f = fopen(filename, "rb");
    if (!f) {
        return STORAGE_ERR_OPEN_FILE;
    }

    sz = fread(buffer, 1, read_size, f);
    fclose(f);

    if (sz < 0) {
        return STORAGE_ERR_READ_FILE;
    }

    return sz;
}

int storage_write(const char *filename, const char *buffer, int write_size)
{
#ifdef __EMSCRIPTEN__
    return -1;
#endif

    FILE *f;
    size_t sz;

    f = fopen(filename, "wb");
    if (!f) {
        return STORAGE_ERR_OPEN_FILE;
    }

    sz = fwrite(buffer, 1, write_size, f);
    fclose(f);

    if (sz != write_size) {
        debug_printf("fwrite write (%d) != expected (%d)\n", sz, write_size);
        return STORAGE_ERR_READ_FILE;
    }

    return STORAGE_OK;
}
