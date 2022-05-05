#ifndef __STORAGE_H
#define __STORAGE_H

#include "types.h"

#define STORAGE_OK 0
#define STORAGE_ERR_NOT_AVAILABLE -1
#define STORAGE_ERR_OPEN_FILE -2
#define STORAGE_ERR_READ_FILE -3
#define STORAGE_ERR_WRITE_FILE -4

void storage_init();

bool storage_available();

int storage_read(const char *filename, char *buffer, int read_offset, int read_size);

int storage_write(const char *filename, const char *buffer, int write_offset, int write_size);

#endif
