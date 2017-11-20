#include "lighdb.h"
#include <stdio.h>

LDB_RES ldb_io_open (LDB_FILE *file, char *path)
{
    return LDB_OK;
}
LDB_RES ldb_io_read (LDB_FILE *file, uint8_t *buf, uint32_t btr, uint32_t *br)
{
    return LDB_OK;
}
LDB_RES ldb_io_write (LDB_FILE *file, uint8_t *buf, uint32_t btw, uint32_t *bw)
{
    return LDB_OK;
}
LDB_RES ldb_io_lseek(LDB_FILE *file, uint32_t offset, int whence)
{
    return LDB_OK;
}
LDB_RES ldb_io_close(LDB_FILE *file)
{
    return LDB_OK;
}
