#include "debug.h"
#include "storage.h"

bool storage_available(void)
{
    return false;
}

int storage_read(const char *filename, char *buffer, int read_size)
{
    (void)filename;
    (void)buffer;
    (void)read_size;
    return STORAGE_ERR_NOT_AVAILABLE;
}

int storage_write(const char *filename, const char *buffer, int write_size)
{
    (void)filename;
    (void)buffer;
    (void)write_size;
    return STORAGE_ERR_NOT_AVAILABLE;
}
