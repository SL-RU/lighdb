#include "lighdb.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

LDB_RES ldb_io_open (LDB_FILE *file, char *path, uint8_t create)
{
    uint32_t flag = FA_READ;
    if(create)
	flag = FA_WRITE | FA_READ;    
    FRESULT res = f_open(file, path, flag);
    
    if(res != FR_OK)
	return LDB_ERR;
    return LDB_OK;
}
LDB_RES ldb_io_read (LDB_FILE *file, uint8_t *buf, uint32_t btr, uint32_t *br)
{
    UINT br = 0;
    FRESULT res = f_read(file, buf, btr, &br);
    if(res != FR_OK)
    {
	printf("r %db f%d:%d\n", br, fd, res);
	return 0;
    }

    ssize_t r = fread(buf, btr, 1, *file);
    if(r != 1)
	return LDB_ERR;
    *br = btr;
    return LDB_OK;
}
LDB_RES ldb_io_write (LDB_FILE *file, uint8_t *buf, uint32_t btw, uint32_t *bw)
{
    ssize_t r = fwrite(buf, btw, 1, *file);
    if(r != 1)
	return LDB_ERR;
    *bw = btw;
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
