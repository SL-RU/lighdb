#include <stdio.h>
#include "lighdb.h"

LighDB db;
uint32_t dbbuf[512/4];
#define ITEM_SIZE 10

int main(int argc, char *argv[])
{
    printf("Create db\n");
    LDB_RES r;
    r = ldb_create(&db,
		   "db.ind", // index file path
		   "db.dat", // data  file path
		   ITEM_SIZE,// size of item
		   0, 0);    // no header
		   
    printf("open result %d\n", r);
    ldb_set_buffer(&db, dbbuf, 512); // set buffer!!!
    uint32_t bs;
    if(r == LDB_OK)
    {
	// add item with and set it's data and ID
	r = ldb_add(&db,
		    "0123456789", // item's data
		    ITEM_SIZE,    // len of item's data buffer
		    13,           // ID of item
		    &bs);         // return's index of item
	r = ldb_add(&db, "asdfdsffdd", ITEM_SIZE, 7, &bs);

        //two items with same ID
	r = ldb_add(&db, "1234567890", ITEM_SIZE, 14, &bs);	
	r = ldb_add(&db, "0987654321", ITEM_SIZE, 14, &bs); 

	uint8_t item[ITEM_SIZE];

        // get item by ID and put it in the buffer
	r = ldb_get(&db,
		    14,        // ID of item
		    item,      // buffer to put data to
		    ITEM_SIZE);// len of buffer
	// update item's data by ID
	r = ldb_upd(&db,
		    14,        // ID of item
		    item,      // buffer with item
		    ITEM_SIZE);// len of buffer		   

	// Index is unique and is index number of item in table.
	// It is value in range [0; db.h.count - 1], ID could be not.
	// If ID is not unique than 
	
	// update item's data by index
	r = ldb_upd_ind(&db,
			0,         // index of item
			item,      // buffer with item
			ITEM_SIZE); // len of buffer
	// get item by index and put it in the buffer
	r = ldb_get_ind(&db,
			2,         // index of item
			item,      // buffer to put data to
			ITEM_SIZE);// len of buffer


	uint32_t indexes[5];
	uint32_t count;
	r = ldb_find_by_id(&db,
			   14,     // ID to find
			   &count, // returns how much was found
			   indexes,// buffer where found indexes will be
			   5);     // len of buffer
	// print found indexes
	printf("found items with ID=14:\n");
	for (int i = 0; i < count; i++) {
	    r = ldb_get_ind(&db, indexes[i], item, ITEM_SIZE);
	    printf("%s\n", item);
	}
	
	r = ldb_close(&db); //close DB
	printf("close result %d\n", r);
    }

    return 0;
}
