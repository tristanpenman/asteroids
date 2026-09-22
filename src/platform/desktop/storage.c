#include <stdio.h>
#include <string.h>

#include "debug.h"
#include "storage.h"

bool storage_available(void)
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

    if (read_size % 32 != 0) {
        debug_printf("Warning: for maximum compatibility, read size should be a multiple of 32");
    }

    memset(buffer, 0, read_size);

    FILE *f = fopen(filename, "rb");
    if (!f) {
        return STORAGE_ERR_OPEN_FILE;
    }

    size_t sz = fread(buffer, 1, read_size, f);
    const bool read_failed = ferror(f) != 0;
    const int close_result = fclose(f);

    if (read_failed || close_result != 0) {
        return STORAGE_ERR_READ_FILE;
    }

    return (int)sz;
}

int storage_write(const char *filename, const char *buffer, int write_size)
{
#ifdef __EMSCRIPTEN__
    return STORAGE_ERR_NOT_AVAILABLE;
#endif

    if (write_size % 32 != 0) {
        debug_printf("Warning: for maximum compatibility, write size should be a multiple of 32");
    }

    FILE *f = fopen(filename, "wb");
    if (!f) {
        return STORAGE_ERR_OPEN_FILE;
    }

    size_t sz = fwrite(buffer, 1, write_size, f);
    const int close_result = fclose(f);

    if (sz != (size_t)write_size || close_result != 0) {
        debug_printf("storage write failed: wrote %zu of %d bytes, fclose returned %d\n",
            sz, write_size, close_result);
        return STORAGE_ERR_WRITE_FILE;
    }

    return STORAGE_OK;
}
