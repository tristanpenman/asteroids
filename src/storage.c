#include <stdio.h>
#include <string.h>

#include "debug.h"
#include "storage.h"

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
    return STORAGE_ERR_NOT_AVAILABLE;
#endif

    FILE *f;
    size_t sz;

    if (read_size % 32 != 0) {
        debug_printf("Warning: for maximum compatibility, read size should be a multiple of 32");
    }

    memset(buffer, 0, read_size);

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
    return STORAGE_ERR_NOT_AVAILABLE;
#endif

    FILE *f;
    size_t sz;

    if (write_size % 32 != 0) {
        debug_printf("Warning: for maximum compatibility, write size should be a multiple of 32");
    }

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
