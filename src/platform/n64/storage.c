#include <nusys.h>
#include <string.h>

#include "storage.h"

#define COMPANY_CODE "ZZ"
#define GAME_CODE "ZZZZ"
#define NOTE_NAME_SIZE 16

static NUContPakFile pak_file;

static bool open_controller_pak(void)
{
    s32 result;

    nuContPakCodeSet(COMPANY_CODE, GAME_CODE);
    nuContQueryRead();
    nuContDataReadStart();
    nuContDataReadWait();

    if (!(nuContStatus[0].status & CONT_CARD_ON)) {
        return false;
    }

    nuContPakOpen(&pak_file, 0);
    if (pak_file.type == NU_CONT_PAK_TYPE_PAK) {
        return true;
    }

    if (!nuContRmbCheck(0)) {
        return false;
    }

    result = nuContPakRepairId(&pak_file);
    if (result != PFS_ERR_NEW_PACK) {
        return false;
    }

    nuContPakOpen(&pak_file, 0);
    return pak_file.type == NU_CONT_PAK_TYPE_PAK;
}

static void make_note_name(char note_name[NOTE_NAME_SIZE], const char *filename)
{
    size_t length = strlen(filename);

    if (length > NOTE_NAME_SIZE) {
        length = NOTE_NAME_SIZE;
    }

    memset(note_name, 0, NOTE_NAME_SIZE);
    memcpy(note_name, filename, length);
}

bool storage_available(void)
{
    return open_controller_pak();
}

int storage_read(const char *filename, char *buffer, int read_size)
{
    char note_name[NOTE_NAME_SIZE];

    if (!storage_available()) {
        return STORAGE_ERR_NOT_AVAILABLE;
    }

    memset(buffer, 0, read_size);
    make_note_name(note_name, filename);

    if (nuContPakFileOpen(&pak_file, note_name, "", NU_CONT_PAK_MODE_NOCREATE, 0)) {
        return STORAGE_ERR_OPEN_FILE;
    }

    nuContPakFileRead(&pak_file, 0, read_size, (u8 *)buffer);
    if (pak_file.error) {
        return STORAGE_ERR_READ_FILE;
    }

    return read_size;
}

int storage_write(const char *filename, const char *buffer, int write_size)
{
    char note_name[NOTE_NAME_SIZE];

    if (!storage_available()) {
        return STORAGE_ERR_NOT_AVAILABLE;
    }

    make_note_name(note_name, filename);

    if (nuContPakFileOpen(&pak_file, note_name, "", NU_CONT_PAK_MODE_CREATE, write_size)) {
        return STORAGE_ERR_OPEN_FILE;
    }

    nuContPakFileWrite(&pak_file, 0, write_size, (u8 *)buffer);
    if (pak_file.error) {
        return STORAGE_ERR_WRITE_FILE;
    }

    return STORAGE_OK;
}
