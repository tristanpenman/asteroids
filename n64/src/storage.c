#include <string.h>

#include <nusys.h>

#include "debug.h"
#include "storage.h"

#define COMPANY_CODE "ZZ"
#define GAME_CODE "ZZZZ"

static NUContPakFile pak_file;

static bool available = false;

/******************************************************************************
 *
 * Helper functions
 *
 *****************************************************************************/

bool open_controller_pak()
{
    s32 ret;

    nuContQueryRead();

    nuContDataReadStart();
    nuContDataReadWait();

    if (nuContStatus[0].status & CONT_CARD_ON) {
        debug_printf(" - nuContStatus says card is on\n");
    } else {
        debug_printf(" - nuContStatus says card is off\n");
        return false;
    }

    nuContPakOpen(&pak_file, 0);

    if (pak_file.type == NU_CONT_PAK_TYPE_PAK) {
        debug_printf(" - nuContPakOpen says card is controller pak\n");
        return true;
    }

    if (!nuContRmbCheck(0)) {
        debug_printf(" - nuContRmbCheck says card is rumble pak\n");
        return false;
    }

    debug_printf(" - nuContPakOpen error: %d\n", pak_file.error);
    debug_printf(" - nuContPakRepairId attempting repair...\n");
    ret = nuContPakRepairId(&pak_file);
    if (ret != PFS_ERR_NEW_PACK) {
        debug_printf(" - nuContPakRepairId failed: %d\n", ret);
        return false;
    }

    // try again
    nuContPakOpen(&pak_file, 0);

    if (pak_file.type == NU_CONT_PAK_TYPE_PAK) {
        debug_printf(" - repair successful\n");
        return true;
    }

    debug_printf(" - repair not successful\n");
    return false;
}

/******************************************************************************
 *
 * Public interface
 *
 *****************************************************************************/

void storage_init()
{
    nuContPakCodeSet(COMPANY_CODE, GAME_CODE);

    available = open_controller_pak();
}

bool storage_available()
{
    return available;
}

int storage_read(const char *filename, char *buffer, int read_size)
{
    s32 err;
    char notename[16];

    if (!available) {
        return STORAGE_ERR_NOT_AVAILABLE;
    }

    memset(buffer, 0, read_size);

    memset(notename, 0, 16);
    memcpy(notename, filename, strlen(filename));

    err = nuContPakFileOpen(&pak_file, notename, "", NU_CONT_PAK_MODE_NOCREATE, 0);
    if (err) {
        return STORAGE_ERR_OPEN_FILE;
    }

    nuContPakFileRead(&pak_file, 0, read_size, (u8*) buffer);
    if (pak_file.error) {
        return STORAGE_ERR_READ_FILE;
    }

    return read_size;
}

int storage_write(const char *filename, const char *buffer, int write_size)
{
    s32 err;
    char notename[16];

    if (!available) {
        return STORAGE_ERR_NOT_AVAILABLE;
    }

    memset(notename, 0, 16);
    memcpy(notename, filename, strlen(filename));

    err = nuContPakFileOpen(&pak_file, notename, "", NU_CONT_PAK_MODE_CREATE, write_size);
    if (err) {
        debug_printf(" - nuContPakFileOpen return: %d\n");
        return STORAGE_ERR_OPEN_FILE;
    }

    nuContPakFileWrite(&pak_file, 0, write_size, (u8*) buffer);
    if (pak_file.error) {
        return STORAGE_ERR_WRITE_FILE;
    }

    return STORAGE_OK;
}
