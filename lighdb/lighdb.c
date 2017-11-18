#include "lighdb.h"

static char ldb_ver[] = "LighDB"LIGHDB_VERSION;

LDB_RES ldb_open(LighDB *db,
		 char *path_index, char *path_data,
		 uint32_t* buffer_size)
{
    if(ldb_io_open(&db->file_index, path_index)) {
	ldb_io_close(&db->file_index);
	return LDB_ERR_IO;
    }
    uint32_t br;
    if(ldb_io_read(&db->file_index,
		   (uint8_t*)&db->h, sizeof(db->h), &br)) {
	ldb_io_close(&db->file_index);
	return LDB_ERR_IO;
    }
    if(sizeof(db->h) != br) {
	ldb_io_close(&db->file_index);
	return LDB_ERR_IO;
    }
    for (uint8_t i = 0; i < 10; i++)
	if(db->h.version[i] != ldb_ver[i])
	{
	    ldb_io_close(&db->file_index);
	    return LDB_ERR_HEADER;
	}
    
    db->index_offset = br + db->h.header_size;
    
    if(ldb_io_open(&db->file_data, path_data)) {
	ldb_io_close(&db->file_data);
	return LDB_ERR_IO;
    }

    *buffer_size = db->h.item_size * 2 + LDB_MIN_ID_BUFF;
    db->opened    = 1;
    db->buffer1   = 0;
    db->buffer2   = 0;
    db->buffer_id = 0;
    db->buffer_id_size = 0;
    return LDB_OK;
}

LDB_RES ldb_close(LighDB *db)
{
}

LDB_RES ldb_set_buffer(LighDB *db, uint8_t *buffer, uint32_t size)
{
    if(size <= db->h.item_size * 2 + LDB_MIN_ID_BUFF)
	return LDB_ERR_SMALL_BUFFER;

    db->buffer1 = buffer;
    db->buffer2 = buffer + db->h.item_size;
    db->buffer_id = buffer + db->h.item_size * 2;
    db->buffer_id_size = size - db->h.item_size * 2;
    return LDB_OK;
}
LDB_RES ldb_create(LighDB *db, char *path_data, char *path_index,
		   uint32_t size,
		   uint32_t header_size, uint8_t *header,
		   uint32_t* buffer_size)
{
}

LDB_RES ldb_get(LighDB *db, uint32_t id,
		uint8_t *buf, uint32_t size)
{
}

LDB_RES ldb_upd(LighDB *db, uint32_t id,
		void *data, uint32_t size)
{
}

LDB_RES ldb_get_ind(LighDB *db, uint32_t index,
		    uint8_t *buf, uint32_t size)
{
}

LDB_RES ldb_upd_ind(LighDB *db, uint32_t index,
		    void *data, uint32_t size)
{
}

LDB_RES ldb_add(LighDB *db,
		void *data, uint32_t size,
		uint32_t id, uint32_t *newindex)
{
}

LDB_RES ldb_find_by_id(LighDB *db, uint32_t *count,
		       uint32_t *list, uint32_t len)
{
}
