/*
  Author: Alexander Lutsai <s.lyra@ya.ru>
  LICENSE: BSD 2-Clause License
*/
#ifndef LIGHDB_H
#define LIGHDB_H

#define LIGHDB_VERSION "001"

#include <stdint.h>

#ifndef LDB_FILE //FILE type
#define LDB_FILE int
#endif

#ifndef LDB_READ_ONLY //will library be readonly
#define LDB_READ_ONLY 0
#endif

#ifndef LDB_MUTEX //will be mutexes used
#define LDB_MUTEX 0
#endif
#if LDB_MUTEX
#define LDB_MUTEX_CREATE(...)
#define LDB_MUTEX_DELETE(...)
#define LDB_MUTEX_REQUEST(...)
#define LDB_MUTEX_RELEASE(...)
#else
#define LDB_MUTEX_CREATE(...)  ldb_mutex_create
#define LDB_MUTEX_DELETE(...)  ldb_mutex_delete
#define LDB_MUTEX_REQUEST(...) ldb_mutex_request_grant
#define LDB_MUTEX_RELEASE(...) ldb_mutex_release_grant
#endif

#ifndef SEEK_SET     //if fcntl.h doesn't included
#define SEEK_SET 0   // Seek relative to begining of file
#define SEEK_CUR 1   // Seek relative to current file position
#define SEEK_END 2   // Seek relative to end of file
#endif

#ifndef LDB_MIN_ID_BUFF //you can change it in settings
#define LDB_MIN_ID_BUFF 256
#endif

typedef enum {
    LDB_OK = 0,          //Everything ok
    LDB_ERR,             //Undefined error
    LDB_ERR_IO,          //Error in IO function
    LDB_BIG_INDEX,       //Index >= total count
    LDB_ERR_NO_ID,       //No ID in ID table
    LDB_ERR_HEADER,      //Error in database header
    LDB_ERR_NO_BUFFER,   //if buffer wasn't been set
    LDB_ERR_NOT_OPENED,  //if db wasn't been opened
    LDB_OK_SMALL_BUFFER, //Small buffer, but it is okay
    LDB_ERR_SMALL_BUFFER,//Small buffer size in argument
    LDB_ERR_ZERO_POINTER,//Zero pointer in arg
} LDB_RES;


//You need to implement IO functions in your project. You can find samples in the implementations folder.
//Or you can set LIGHDB_USE_STDIO
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
#if !LDB_READ_ONLY
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
#endif
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


//Database consists from two files: one with item's data one by one, other with header, some values and table of ID's for each data item
/*
  Index file structure:

  |LightDB version(10bytes)|header_size(4bytes)|item_size(4bytes)|count(4bytes)|header(header_size bytes)|table of id(count*4 bytes)|
  Data file structure:
  |LightDB version(10bytes)|item's data one by one(item_size * count bytes)|
*/
/*
  INDEX is unique and it defines index in data array
  ID can be not unique and just defines link between INDEX and some number
 */

typedef struct {
    uint8_t opened;
    
    LDB_FILE file_data;   //file with data
    LDB_FILE file_index;   //file with index

    struct {
	char version[10];
	uint32_t header_size; //size of header
	uint32_t item_size;   //size of single item
	uint32_t count;       //total count of items
    } h;
    
    uint32_t index_offset; //offset of ID data in file_index
    uint32_t data_offset; //offset of data in file_data
    //buffers
    uint32_t *buffer_id;
    uint32_t buffer_id_size;
    
#if LDB_MUTEX
    LDB_MUTEX_t mutex; //mutex if enabled
#endif
} LighDB;



/**
 * Open existing DB
 *
 * @param db pointer to DB structure
 * @param path_index path to index file of DB
 * @param path_data path to data file of DB
 * @param buffer_size returns buffer size for DB functionality. You need to set buffer using ldb_set_buffer(); Size equals [N] bytes. N byte for buffer of indexes, more is better. But not less then LDB_MIN_ID_BUFF bytes
 * @return result LDB_OK, LDB_ERR, LDB_ERR_IO
 */
LDB_RES ldb_open(LighDB *db,
		 char *path_index, char *path_data,
		 uint32_t* buffer_size);

/**
 * Close opened DB. After DB buffer doesn't required anymore.
 *
 * @param db pointer to DB structure
 * @return result LDB_OK, LDB_ERR_IO
 */
LDB_RES ldb_close(LighDB *db);
/**
 * Set buffer. 
 *
 * @param db pointer to DB structure 
 * @param buffer buffer
 * @param size size of buffer in bytes
 * @retur result LDB_OK, LDB_ERR_SMALL_BUFFER
 */
LDB_RES ldb_set_buffer(LighDB *db, uint32_t *buffer, uint32_t size);
#if !LDB_READ_ONLY
/**
 * Create new database
 *
 * @param db pointer to DB structure
 * @param path_data path to data DB file
 * @param path_index path to index DB file
 * @param size size of a single item's data
 * @param header_size size of header
 * @param header header buffer
 * @param buffer_size returns buffer size for DB functionality. You need to set buffer using ldb_set_buffer(); Size equals [N] bytes. N byte for buffer of indexes, more is better. But not less then LDB_MIN_ID_BUFF bytes
 * @return result LDB_OK, LDB_ERR_IO
 */
LDB_RES ldb_create(LighDB *db, char *path_data, char *path_index,
		   uint32_t size,
		   uint32_t header_size, uint8_t *header,
		   uint32_t* buffer_size);
#endif
/**
 * Get data from first found item by ID
 *
 * @param db pointer to DB structure
 * @param id ID of the data 
 * @param buf buffer of data
 * @param size size of data
 * @return result LDB_OK, LDB_ERR_IO, LDB_ERR_SMALL_BUFFER
 */
LDB_RES ldb_get(LighDB *db, uint32_t id,
		uint8_t *buf, uint32_t size);
#if !LDB_READ_ONLY
/**
 * Change data on first found item by ID
 *
 * @param db pointer to DB structure
 * @param id ID of the data 
 * @param buf buffer of data
 * @param size size of data
 * @return result LDB_OK, LDB_ERR_IO, LDB_ERR_SMALL_BUFFER
 */
LDB_RES ldb_upd(LighDB *db, uint32_t id,
		void *data, uint32_t size);
#endif
/**
 * Get data from item by index
 *
 * @param db pointer to DB structure
 * @param index index of item
 * @param buf buffer of data
 * @param size size of data
 * @return result LDB_OK, LDB_ERR_IO, LDB_ERR_SMALL_BUFFER
 */
LDB_RES ldb_get_ind(LighDB *db, uint32_t index,
		    uint8_t *buf, uint32_t size);
#if !LDB_READ_ONLY
/**
 * Change item's data by index
 *
 * @param db pointer to DB structure
 * @param index index of row 
 * @param buf buffer of data
 * @param size size of data
 * @return result LDB_OK, LDB_ERR_IO, LDB_ERR_SMALL_BUFFER
 */
LDB_RES ldb_upd_ind(LighDB *db, uint32_t index,
		    void *data, uint32_t size);
/**
 * Add new item
 *
 * @param db pointer to DB structure
 * @param data data of item
 * @param size size of data
 * @param id ID of new item
 * @param newindex returns index of new item
 * @return result LDB_OK, LDB_ERR_IO, LDB_ERR_SMALL_BUFFER
 */
LDB_RES ldb_add(LighDB *db,
		void *data, uint32_t size,
		uint32_t id, uint32_t *newindex);
#endif
/**
 * Get count of indexes of items with selected ID. And if list != 0 && len != 0 then put found indexes in the list
 *
 * @param db pointer to DB structure
 * @param count returns count of found indexes or equals len
 * @param list array with found indexes. Can be 0.
 * @param len length of array. Can be 0. If len>0 and  (real count)<len then count will be equal to len and will be returned LDB_OK_SMALL_BUFFER.
 * @return result LDB_OK, LDB_ERR_IO, LDB_OK_SMALL_BUFFER
 */
LDB_RES ldb_find_by_id(LighDB *db, uint32_t id,
		       uint32_t *count,
		       uint32_t *list, uint32_t len);
LDB_RES ldb_get_header(LighDB *db, uint8_t *buf, uint32_t size, uint32_t *readed);
#if !LDB_READ_ONLY
LDB_RES ldb_set_header(LighDB *db, uint8_t *buf, uint32_t size, uint32_t *written);
#endif
#endif






