#ifndef LIGHDB_H
#define LIGHDB_H

#include <stdint.h>

#if LIGHDB_USE_STDIO
#define LDB_FILE int
#endif

#ifndef LDB_FILE //FILE type
#define LDB_FILE int
#endif

#ifndef SEEK_SET     //if fcntl.h doesn't included
#define SEEK_SET 0   // Seek relative to begining of file
#define SEEK_CUR 1   // Seek relative to current file position
#define SEEK_END 2   // Seek relative to end of file
#endif


typedef enum {
    LDB_OK = 0,
    LDB_ERR
} LDB_RES;


//You need to implement IO functions in your project. You can find samples in the implementations folder.
//Or you 
/**
 * Open file
 *
 * @param file file object or descriptor
 * @param path path to file
 * @return result LDB_OK or LDB_ERR
 */
LDB_RES ldb_io_open (LDB_FILE *file, char *path);
/**
 * Read data from file to buffer
 *
 * @param file file object or descriptor
 * @param buf buffer
 * @param btr count of bytes to read. Must be less or equal to buffer len
 * @param br total bytes read
 * @return result LDB_OK or LDB_ERR
 */
LDB_RES ldb_io_read (LDB_FILE *file, uint8_t *buf, uint32_t btr, uint32_t *br);
/**
 * Write data to file from buffer
 *
 * @param file file object or descriptor
 * @param buf buffer
 * @param btr count of bytes to write. Must be less or equal to buffer len
 * @param br total bytes written
 * @return result LDB_OK or LDB_ERR
 */
LDB_RES ldb_io_write (LDB_FILE *file, uint8_t *buf, uint32_t btw, uint32_t *bw);
/**
 * Seek position in the buffer. Similar to std lseek
 *
 * @param file file object or descriptor
 * @param offset offset in bytes
 * @param whence SEEK_SET, SEEK_CUR, SEEK_END
 * @return result LDB_OK or LDB_ERR
 */
LDB_RES ldb_io_lseek(LDB_FILE *file, uint32_t offset, int whence);
/**
 * Close file
 *
 * @param file file object or descriptor
 * @return result LDB_OK or LDB_ERR
 */
LDB_RES ldb_io_close(LDB_FILE *file);


typedef struct {
LDB_FILE file_index;
LDB_FILE file_table;
uint32_t columns_num;
uint32_t * columns_width;
uint32_t index_offset;

} LighDB;



LDB_RES ldb_open();



#endif
