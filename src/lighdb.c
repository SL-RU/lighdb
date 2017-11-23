#include "lighdb.h"

static char ldb_ver[] = "LighDB"LIGHDB_VERSION;

LDB_RES ldb_open(LighDB *db,
		 char *path_index, char *path_data,
		 uint32_t* buffer_size)
{
    if(db == 0 || path_index == 0 || path_data == 0 || buffer_size == 0)
	return LDB_ERR_ZERO_POINTER;

    //open index file
    if(ldb_io_open(&db->file_index, path_index, 0)) {
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
    db->index_offset = sizeof(db->h) + db->h.header_size;
    //open data file
    if(ldb_io_open(&db->file_data, path_data, 0)) {
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

    printf("%s %ld it sz %d, count %d\n", db->h.version, sizeof(db->h), db->h.item_size, db->h.count);
    
    //calculate buffer size
    *buffer_size = LDB_MIN_ID_BUFF;
    db->data_offset = 10;
    //clear buffer pointers
    db->buffer_id = 0;
    db->buffer_id_size = 0;

    if(LDB_MUTEX_CREATE(&db->mutex))
	return LDB_ERR_MUTEX;
    
    return LDB_OK;
}

LDB_RES ldb_close(LighDB *db)
{
    if(db == 0)
	return LDB_ERR_ZERO_POINTER;
    if(LDB_MUTEX_REQUEST(&db->mutex))  //reQuest MUTEX
	return LDB_ERR_MUTEX;	
    if(db->opened == 0)
    {
	LDB_MUTEX_RELEASE(&db->mutex); //reLease MUTEX
	return LDB_ERR_NOT_OPENED;
    }
    db->opened    = 0;
    db->buffer_id = 0;
    db->buffer_id_size = 0;

    if(ldb_io_close(&db->file_index)) {
	ldb_io_close(&db->file_data);
	LDB_MUTEX_RELEASE(&db->mutex); //reLease MUTEX
	return LDB_ERR_IO;
    }
    if(ldb_io_close(&db->file_data))
    {
	LDB_MUTEX_RELEASE(&db->mutex); //reLease MUTEX
	return LDB_ERR_IO;
    }
    if(LDB_MUTEX_RELEASE(&db->mutex))  //reLease MUTEX
	return LDB_ERR_MUTEX;	
    if(LDB_MUTEX_DELETE(&db->mutex))
	return LDB_ERR_MUTEX;	
    return LDB_OK;
}

LDB_RES ldb_set_buffer(LighDB *db, uint32_t *buffer, uint32_t size)
{
    if(db == 0 || buffer == 0)
	return LDB_ERR_ZERO_POINTER;
    if(size < LDB_MIN_ID_BUFF)
	return LDB_ERR_SMALL_BUFFER;
    if(LDB_MUTEX_REQUEST(&db->mutex))   //reQuest MUTEX
	return LDB_ERR_MUTEX;	
    if(db->opened == 0)
    {
	LDB_MUTEX_RELEASE(&db->mutex);  //reLease MUTEX
	return LDB_ERR_NOT_OPENED;
    }
    db->buffer_id = (uint32_t*)buffer;
    db->buffer_id_size = size;
    if(LDB_MUTEX_RELEASE(&db->mutex))   //reLease MUTEX
	return LDB_ERR_MUTEX;	
    return LDB_OK;
}
#if !LDB_READ_ONLY
static LDB_RES update_sysheader(LighDB *db)
{
    if(ldb_io_lseek(&db->file_index, 0, SEEK_SET))
	return LDB_ERR_IO;
    uint32_t bw;
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
   
    return LDB_OK;
}
LDB_RES ldb_create(LighDB *db, char *path_index, char *path_data,
		   uint32_t size,
		   uint32_t header_size, uint8_t *header,
		   uint32_t* buffer_size)
{
    if(db == 0 || path_index == 0 || path_data == 0 || buffer_size == 0)
	return LDB_ERR_ZERO_POINTER;
    if(size == 0)
	return LDB_ERR;

    //open index file
    if(ldb_io_open(&db->file_index, path_index, 1)) {
	
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
    LDB_RES r;
    //write db header
    if((r = update_sysheader(db)))
	return r;
    if(header != 0 && header_size != 0)
    {
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
    }
    
    //open data file
    if(ldb_io_open(&db->file_data, path_data, 1)) {
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
    db->index_offset = sizeof(db->h) + header_size;
    db->data_offset = 10;
    
    //calculate buffer size
    *buffer_size = LDB_MIN_ID_BUFF;
    //clear buffer pointers
    db->buffer_id = 0;
    db->buffer_id_size = 0;

    if(LDB_MUTEX_CREATE(&db->mutex))
	return LDB_ERR_MUTEX;	
    
    return LDB_OK;
}
#endif
inline static LDB_RES chk_db(LighDB *db)
{
    if(db == 0)
	return LDB_ERR_ZERO_POINTER;
    if(LDB_MUTEX_REQUEST(&db->mutex)) //reQuest MUTEX
	return LDB_ERR_MUTEX;	
    if(db->opened == 0)
    {
	LDB_MUTEX_RELEASE(&db->mutex);//reLease MUTEX
	return LDB_ERR_NOT_OPENED;
    }
    if(db->buffer_id == 0)
    {
	LDB_MUTEX_RELEASE(&db->mutex);//reLease MUTEX
	return LDB_ERR_NO_BUFFER;
    }
    return LDB_OK;
}
LDB_RES ldb_get(LighDB *db, uint32_t id,
		uint8_t *buf, uint32_t size)
{
    LDB_RES r;
    if((r = chk_db(db)))               //reQuest MUTEX
	return r;

    uint32_t index, count;
    //find first element with ID
    if(LDB_MUTEX_RELEASE(&db->mutex))  //reLease MUTEX
	return LDB_ERR_MUTEX;
    r = ldb_find_by_id(db, id, &count, &index, 1);
    if(LDB_MUTEX_REQUEST(&db->mutex))  //reQuest MUTEX
	return LDB_ERR_MUTEX;	
    if(r != LDB_OK && r != LDB_OK_SMALL_BUFFER)
    {
	LDB_MUTEX_RELEASE(&db->mutex); //reLease MUTEX
	return LDB_ERR_NO_ID;
    }
    if(count == 0) //if 0 elements found
    {
	LDB_MUTEX_RELEASE(&db->mutex); //reLease MUTEX
	return LDB_ERR_NO_ID;
    }
    if(LDB_MUTEX_RELEASE(&db->mutex))  //reLease MUTEX
	return LDB_ERR_MUTEX;	
    return ldb_get_ind(db, index, buf, size);
}

#if !LDB_READ_ONLY
LDB_RES ldb_upd(LighDB *db, uint32_t id,
		void *data, uint32_t size)
{
    LDB_RES r;
    if((r = chk_db(db)))              //reQuest MUTEX
	return r;
    
    uint32_t index, count;
    //find first element with ID
    if(LDB_MUTEX_RELEASE(&db->mutex)) //reLease MUTEX
	return LDB_ERR_MUTEX;	
    r = ldb_find_by_id(db, id, &count, &index, 1);
    if(LDB_MUTEX_REQUEST(&db->mutex)) //reQuest MUTEX
	return LDB_ERR_MUTEX;	
    if(r != LDB_OK && r != LDB_OK_SMALL_BUFFER)
    {
	LDB_MUTEX_RELEASE(&db->mutex);//reLease MUTEX
	return LDB_ERR_NO_ID;
    }
    if(count == 0) //if 0 elements found
    {
	LDB_MUTEX_RELEASE(&db->mutex);//reLease MUTEX
	return LDB_ERR_NO_ID;
    }
    if(LDB_MUTEX_RELEASE(&db->mutex)) //reLease MUTEX
	return LDB_ERR_MUTEX;	
    
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
    if(buf == 0)
	return LDB_ERR_ZERO_POINTER;
    if((r = chk_db(db)))              //reQuest MUTEX
	return r;
    if(size < db->h.item_size)
    {
	LDB_MUTEX_RELEASE(&db->mutex);//reLease MUTEX
	return LDB_ERR_SMALL_BUFFER;
    }
    
    if((r = ldb_seek_ind_data(db, index)))
	return r;
    uint32_t br;
    if(ldb_io_read(&db->file_data, buf, db->h.item_size, &br))
    {
	LDB_MUTEX_RELEASE(&db->mutex);//reLease MUTEX
	return LDB_ERR_IO;
    }
    return LDB_OK;
}
#if !LDB_READ_ONLY
LDB_RES ldb_upd_ind(LighDB *db, uint32_t index,
		    void *data, uint32_t size)
{
    LDB_RES r;
    if(data == 0)
	return LDB_ERR_ZERO_POINTER;
    if((r = chk_db(db)))              //reQuest MUTEX
	return r;
    if(size < db->h.item_size)
    {
	LDB_MUTEX_RELEASE(&db->mutex);//reLease MUTEX
	return LDB_ERR_SMALL_BUFFER;
    }
    if((r = ldb_seek_ind_data(db, index)))
    {
	LDB_MUTEX_RELEASE(&db->mutex);//reLease MUTEX
	return r;
    }
    uint32_t bw;
    if(ldb_io_write(&db->file_data, data, db->h.item_size, &bw))
    {
	LDB_MUTEX_RELEASE(&db->mutex);//reLease MUTEX
	return LDB_ERR_IO;
    }
    if(LDB_MUTEX_RELEASE(&db->mutex))
	return LDB_ERR_MUTEX;	      //reLease MUTEX
    return LDB_OK;    
}

LDB_RES ldb_add(LighDB *db,
		void *data, uint32_t size,
		uint32_t id, uint32_t *newindex)
{
    LDB_RES r;
    if(data == 0)
	return LDB_ERR_ZERO_POINTER;
    if((r = chk_db(db)))              //reQuest MUTEX
    	return r;
    if(size < db->h.item_size) {
	LDB_MUTEX_RELEASE(&db->mutex);//reLease MUTEX
	return LDB_ERR_SMALL_BUFFER;
    }

    //add to data
    if(ldb_io_lseek(&db->file_data,
		    db->data_offset +
		    (db->h.item_size * db->h.count),
		    SEEK_SET)) {
	LDB_MUTEX_RELEASE(&db->mutex);//reLease MUTEX
	return LDB_ERR_IO;
    }
    uint32_t bw;
    if(ldb_io_write(&db->file_data, data, db->h.item_size, &bw)) {
	LDB_MUTEX_RELEASE(&db->mutex);//reLease MUTEX
	return LDB_ERR_IO;
    }
    //add in ID table
    if(ldb_io_lseek(&db->file_index,
		    db->index_offset +
		    (4 * db->h.count),
		    SEEK_SET)) {
	LDB_MUTEX_RELEASE(&db->mutex);//reLease MUTEX
	return LDB_ERR_IO;
    }
    if(ldb_io_write(&db->file_index, (uint8_t*)&id, sizeof(uint32_t), &bw)) {
	LDB_MUTEX_RELEASE(&db->mutex);//reLease MUTEX
	return LDB_ERR_IO;
    }

    //return new index
    if(newindex != 0)
	*newindex = db->h.count;
    
    db->h.count ++;
    //update count in db header
    if((r = update_sysheader(db))) {
	LDB_MUTEX_RELEASE(&db->mutex);//reLease MUTEX
	return r;
    }

    //insert id in ID table
    if(db->buffer_id_count < db->buffer_id_size &&
       db->buffer_id_start_index + db->buffer_id_size < db->buffer_id_count)
    {
	db->buffer_id[db->buffer_id_count] = id;
    }
    if(LDB_MUTEX_RELEASE(&db->mutex)) //reLease MUTEX
	return LDB_ERR_MUTEX;	

    return LDB_OK;    
}
#endif
static LDB_RES load_buf(LighDB *db, uint32_t sind)
{
    if(sind >= db->h.count)
	return LDB_ERR;
    
    db->buffer_id_start_index = sind; //set first index
    //calculate count
    db->buffer_id_count = db->h.count - sind; 
    if(db->buffer_id_count > db->buffer_id_size)
	db->buffer_id_count = db->buffer_id_size;
    //load table
    uint32_t br;
    ldb_io_lseek(&db->file_index,
		 db->index_offset + db->buffer_id_start_index * 4,
		 SEEK_SET);
    ldb_io_read(&db->file_index, (uint8_t*)db->buffer_id,
		db->buffer_id_count * 4, &br);
    if(br != db->buffer_id_count * 4)
	return LDB_ERR_IO;
    
    return LDB_OK;
}
LDB_RES ldb_find_by_id(LighDB *db, uint32_t id,
		       uint32_t *count,
		       uint32_t *list, uint32_t len)
{
    LDB_RES r;
    if(len == 0)
	return LDB_OK;
    if((r = chk_db(db)))                  //reQuest MUTEX
    	return r;

    uint32_t l = db->h.count; //how many indexes left
    uint32_t i, was_zero;
    
    if(db->buffer_id_count == 0)
	if((r = load_buf(db, 0))) {
	    LDB_MUTEX_RELEASE(&db->mutex);//reLease MUTEX
	    return r;
	}

    (*count) = 0;
    
    //how many indexes left
    was_zero = (db->buffer_id_start_index == 0);
    
    do {
	for (i = 0; i < db->buffer_id_count; i++) {
	    if(db->buffer_id[i] == id)
	    {
		if(list != 0 && len > (*count))
		{
		    list[(*count)] = i + db->buffer_id_start_index;
		    if(len == (*count) + 1)
		    {
			(*count) ++;
			//reLease MUTEX
			LDB_MUTEX_RELEASE(&db->mutex);
			return LDB_OK;
		    }
		}
    
		(*count) ++;
	    }
	}
	//load next sheet of ID's table
	if(!was_zero) {
	    //if zero index wasn't in buffer. Load first index
	    load_buf(db, 0);
	    was_zero = 1;
	    l = db->h.count;
	} else {
	    //load next sheet
	    l -= db->buffer_id_count;   
	    if(load_buf(db,
			db->buffer_id_start_index +
			db->buffer_id_count))
		break;
	}
    } while(l != 0);

    if(LDB_MUTEX_RELEASE(&db->mutex)) //reLease MUTEX
	return LDB_ERR_MUTEX;	

    return LDB_OK;
}
LDB_RES ldb_get_header(LighDB *db,
		       uint8_t *buf, uint32_t size,
		       uint32_t *read)
{
    LDB_RES r;
    if(buf == 0 || size == 0)
	return LDB_ERR_ZERO_POINTER;
    if((r = chk_db(db)))              //reQuest MUTEX
	return r;

    uint32_t br;
    if(size > db->h.header_size)
	size = db->h.header_size;
    //read user header
    if(ldb_io_read(&db->file_index,
		    buf, size, &br)) {
	LDB_MUTEX_RELEASE(&db->mutex);//reLease MUTEX
	return LDB_ERR_IO;
    }
    //check read user header size
    if(size != br) {
	LDB_MUTEX_RELEASE(&db->mutex);//reLease MUTEX
	return LDB_ERR_IO;
    }
    if(LDB_MUTEX_RELEASE(&db->mutex)) //reLease MUTEX
	return LDB_ERR_MUTEX;	

    if(read != 0)
	*read = 0;
    
    return LDB_OK;
}
#if !LDB_READ_ONLY
LDB_RES ldb_set_header(LighDB *db, uint8_t *buf, uint32_t size, uint32_t *written)
{
    LDB_RES r;
    if(buf == 0 || size == 0)
	return LDB_ERR_ZERO_POINTER;
    if((r = chk_db(db)))              //reQuest MUTEX
	return r;

    uint32_t bw;
    if(size > db->h.header_size)
	size = db->h.header_size;
    //write user header
    if(ldb_io_write(&db->file_index,
		    buf, size, &bw)) {
	LDB_MUTEX_RELEASE(&db->mutex);//reLease MUTEX
	return LDB_ERR_IO;
    }
    //check written user header size
    if(size != bw) {
	LDB_MUTEX_RELEASE(&db->mutex);//reLease MUTEX
	return LDB_ERR_IO;
    }
    if(LDB_MUTEX_RELEASE(&db->mutex)) //reLease MUTEX
	return LDB_ERR_MUTEX;	

    if(written != 0)
	*written = size;
    
    return LDB_OK;
}
#endif
#if LDB_MUTEX == 0
LDB_RES ldb_return_ok(LDB_MUTEX_t *x) {
    //REMOVE F**** unused varible
    return LDB_OK + ((long int)x != (long int)x*1);
}
#endif
