#include "lighdb.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

LDB_RES ldb_io_open (LDB_FILE *file, char *path, uint8_t create)
{
    if(create)
	*file = fopen(path, "w+b");
    else
	*file = fopen(path, "r+b");
    
    if(*file == NULL)
	return LDB_ERR;
    return LDB_OK;
}
LDB_RES ldb_io_read (LDB_FILE *file, uint8_t *buf, uint32_t btr, uint32_t *br)
{
    ssize_t r = fread(buf, 1, btr, *file);
    if(r != btr)
	return LDB_ERR;
    *br = r;
    return LDB_OK;
}
LDB_RES ldb_io_write (LDB_FILE *file, uint8_t *buf, uint32_t btw, uint32_t *bw)
{
    ssize_t r = fwrite(buf, 1, btw, *file);
    if(r < 0)
	return LDB_ERR;
    *bw = r;
    return LDB_OK;
}
LDB_RES ldb_io_lseek(LDB_FILE *file, uint32_t offset, int whence)
{
    ssize_t r = fseek(*file, offset, whence);
    if(r < 0)
	return LDB_ERR;
    return LDB_OK;
}
LDB_RES ldb_io_close(LDB_FILE *file)
{
    if(fclose(*file) == EOF)
	return LDB_ERR;
    return LDB_OK;
}
