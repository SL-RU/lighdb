#include "lighdb.h"

static char ldb_ver[] = "LighDB"LIGHDB_VERSION;

LDB_RES ldb_open(LighDB *db,
		 char *path_index, char *path_data,
		 uint32_t* buffer_size)
{
    if(db == 0 || path_index == 0 || path_data == 0 || buffer_size == 0)
	return LDB_ERR_ZERO_POINTER;

    //open index file
    if(ldb_io_open(&db->file_index, path_index)) {
	ldb_io_close(&db->file_index);
	return LDB_ERR_IO;
    }
    //set db opened
    db->opened    = 1;
    
    uint32_t br;
    //read header
    if(ldb_io_read(&db->file_index,
		   (uint8_t*)&db->h, sizeof(db->h), &br)) {
	ldb_io_close(&db->file_index);
	return LDB_ERR_IO;
    }
    //check header size
    if(sizeof(db->h) != br) {
	ldb_io_close(&db->file_index);
	return LDB_ERR_IO;
    }
    //check is it LighDB
    for (uint8_t i = 0; i < 6; i++)
	if(db->h.version[i] != ldb_ver[i])
	{
	    ldb_io_close(&db->file_index);
	    return LDB_ERR_HEADER;
	}
    //calculate index table offset
    db->index_offset = br + db->h.header_size;
    //open data file
    if(ldb_io_open(&db->file_data, path_data)) {
	ldb_io_close(&db->file_data);
	return LDB_ERR_IO;
    }

    uint8_t buf[10];
    //read first 10 bytes in data file
    if(ldb_io_read(&db->file_data,
		   buf, 10, &br)) {
	ldb_io_close(&db->file_index);
	ldb_io_close(&db->file_data);
	return LDB_ERR_IO;
    }
    //check count
    if(10 != br) {
	ldb_io_close(&db->file_index);
	ldb_io_close(&db->file_data);
	return LDB_ERR_IO;
    }
    //check is it LighDB
    for (uint8_t i = 0; i < 6; i++)
	if(buf[i] != ldb_ver[i])
	{
	    ldb_io_close(&db->file_index);
	    ldb_io_close(&db->file_data);
	    return LDB_ERR_HEADER;
	}
    
    //calculate buffer size
    *buffer_size = LDB_MIN_ID_BUFF;
    db->data_offset = 10;
    //clear buffer pointers
    db->buffer_id = 0;
    db->buffer_id_size = 0;
    return LDB_OK;
}

LDB_RES ldb_close(LighDB *db)
{
    if(db == 0)
	return LDB_ERR_ZERO_POINTER;
    if(db->opened == 0)
	return LDB_ERR_NOT_OPENED;
    db->opened    = 0;
    db->buffer_id = 0;
    db->buffer_id_size = 0;

    if(ldb_io_close(&db->file_index)) {
	ldb_io_close(&db->file_data);
	return LDB_ERR_IO;
    }
    if(ldb_io_close(&db->file_data))
	return LDB_ERR_IO;
    
    return LDB_OK;
}

LDB_RES ldb_set_buffer(LighDB *db, uint32_t *buffer, uint32_t size)
{
    if(db == 0 || buffer == 0)
	return LDB_ERR_ZERO_POINTER;
    if(db->opened == 0)
	return LDB_ERR_NOT_OPENED;

    if(size <= LDB_MIN_ID_BUFF)
	return LDB_ERR_SMALL_BUFFER;

    db->buffer_id = (uint32_t*)buffer;
    db->buffer_id_size = size;
    return LDB_OK;
}
#if !LDB_READ_ONLY
LDB_RES ldb_create(LighDB *db, char *path_data, char *path_index,
		   uint32_t size,
		   uint32_t header_size, uint8_t *header,
		   uint32_t* buffer_size)
{
    if(db == 0 || path_index == 0 || path_data == 0 || buffer_size == 0)
	return LDB_ERR_ZERO_POINTER;
    if(size == 0)
	return LDB_ERR;

    //open index file
    if(ldb_io_open(&db->file_index, path_index)) {
	ldb_io_close(&db->file_index);
	return LDB_ERR_IO;
    }
    //set db opened
    db->opened    = 1;
    //copy version
    for (uint8_t i = 0; i < 10; i++)
	db->h.version[i] = ldb_ver[i];
    //set header
    db->h.header_size = header_size;
    db->h.item_size = size;
    db->h.count = 0;
    
    uint32_t bw;
    //write db header
    if(ldb_io_write(&db->file_index,
		    (uint8_t*)&db->h, sizeof(db->h), &bw)) {
	ldb_io_close(&db->file_index);
	return LDB_ERR_IO;
    }
    //check written db header size
    if(sizeof(db->h) != bw) {
	ldb_io_close(&db->file_index);
	return LDB_ERR_IO;
    }
    //write user header
    if(ldb_io_write(&db->file_index,
		    header, header_size, &bw)) {
	ldb_io_close(&db->file_index);
	return LDB_ERR_IO;
    }
    //check written user header size
    if(header_size != bw) {
	ldb_io_close(&db->file_index);
	return LDB_ERR_IO;
    }
    
    //open data file
    if(ldb_io_open(&db->file_data, path_data)) {
	ldb_io_close(&db->file_data);
	return LDB_ERR_IO;
    }
    //write version in data file
    if(ldb_io_write(&db->file_data,
		    (uint8_t*)ldb_ver, 10, &bw)) {
	ldb_io_close(&db->file_index);
	ldb_io_close(&db->file_data);
	return LDB_ERR_IO;
    }

    //calculate index table offset
    db->index_offset = bw + header_size;
    db->data_offset = 10;
    
    //calculate buffer size
    *buffer_size = db->h.item_size * 2 + LDB_MIN_ID_BUFF;
    //clear buffer pointers
    db->buffer_id = 0;
    db->buffer_id_size = 0;
    return LDB_OK;
}
#endif
inline static LDB_RES chk_db(LighDB *db)
{
    if(db == 0)
	return LDB_ERR_ZERO_POINTER;
    if(db->opened == 0)
	return LDB_ERR_NOT_OPENED;
    if(db->buffer_id == 0)
	return LDB_ERR_NO_BUFFER;
    return LDB_OK;
}
LDB_RES ldb_get(LighDB *db, uint32_t id,
		uint8_t *buf, uint32_t size)
{
    LDB_RES r;
    if((r = chk_db(db)))
	return r;

    uint32_t index, count;
    //find first element with ID
    r = ldb_find_by_id(db, id, &count, &index, 1);
    if(r != LDB_OK && r != LDB_OK_SMALL_BUFFER)
	return LDB_ERR_NO_ID;
    if(count == 0) //if 0 elements found
	return LDB_ERR_NO_ID;
    
    return ldb_get_ind(db, index, buf, size);
}

#if !LDB_READ_ONLY
LDB_RES ldb_upd(LighDB *db, uint32_t id,
		void *data, uint32_t size)
{
    LDB_RES r;
    if((r = chk_db(db)))
	return r;
    
    uint32_t index, count;
    //find first element with ID
    r = ldb_find_by_id(db, id, &count, &index, 1);
    if(r != LDB_OK && r != LDB_OK_SMALL_BUFFER)
	return LDB_ERR_NO_ID;
    if(count == 0) //if 0 elements found
	return LDB_ERR_NO_ID;
    
    return ldb_upd_ind(db, index, data, size);
}
#endif
static LDB_RES ldb_seek_ind_data(LighDB *db, uint32_t index)
{
    if(index >= db->h.count)
	return LDB_BIG_INDEX;
    if(ldb_io_lseek(&db->file_data,
		    db->data_offset +
		    (db->h.item_size * index),
		    SEEK_SET))
	return LDB_ERR_IO;
    return LDB_OK;
}
LDB_RES ldb_get_ind(LighDB *db, uint32_t index,
		    uint8_t *buf, uint32_t size)
{
    LDB_RES r;
    if((r = chk_db(db)))
	return r;
    if(buf == 0)
	return LDB_ERR_ZERO_POINTER;
    if(size < db->h.item_size)
	return LDB_ERR_SMALL_BUFFER;

    
    if((r = ldb_seek_ind_data(db, index)))
	return r;
    uint32_t br;
    if(ldb_io_read(&db->file_data, buf, db->h.item_size, &br))
	return LDB_ERR_IO;

    return LDB_OK;
}
#if !LDB_READ_ONLY
LDB_RES ldb_upd_ind(LighDB *db, uint32_t index,
		    void *data, uint32_t size)
{
    LDB_RES r;
    if((r = chk_db(db)))
	return r;
    if(data == 0)
	return LDB_ERR_ZERO_POINTER;
    if(size < db->h.item_size)
	return LDB_ERR_SMALL_BUFFER;

    if((r = ldb_seek_ind_data(db, index)))
	return r;
    uint32_t bw;
    if(ldb_io_write(&db->file_data, data, db->h.item_size, &bw))
	return LDB_ERR_IO;
    
    return LDB_OK;    
}

LDB_RES ldb_add(LighDB *db,
		void *data, uint32_t size,
		uint32_t id, uint32_t *newindex)
{
    LDB_RES r;
    if((r = chk_db(db)))
    	return r;
    if(data == 0)
	return LDB_ERR_ZERO_POINTER;
    if(size < db->h.item_size)
	return LDB_ERR_SMALL_BUFFER;

    if(ldb_io_lseek(&db->file_data,
		    db->data_offset +
		    (db->h.item_size * db->h.count),
		    SEEK_SET))
	return LDB_ERR_IO;
    uint32_t bw;
    if(ldb_io_write(&db->file_data, data, db->h.item_size, &bw))
	return LDB_ERR_IO;

    
    
    return LDB_OK;    
}
#endif
LDB_RES ldb_find_by_id(LighDB *db, uint32_t *count,
		       uint32_t *list, uint32_t len)
{
}
