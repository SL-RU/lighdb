#include "lighdb.h"

LighDB db;
uint32_t dbbuf[512/4];

int main(int argc, char *argv[])
{
    printf("Create db\n");
    LDB_RES r;
    r = ldb_create(&db,
		   "db.ind", // index file path
		   "db.dat", // data  file path
		   10,       // size of item
		   0, 0);    // no header
		   
    printf("open result %d\n", r);
    ldb_set_buffer(&db, dbbuf, 512); // set buffer!!!
    uint32_t bs;
    if(r == LDB_OK)
    {
	r = ldb_add(&db,
		    "0123456789", // item's data
		    10, // len of item's data buffer
		    13, // ID of item
		    &bs); //return's index of item
	printf("add %d\nnew index: %d\n", r, bs);
	r = ldb_add(&db, "asdfdsffdd", 10, 7, &bs);
	printf("add %d\nnew index: %d\n", r, bs);
	r = ldb_add(&db, "1234567890", 10, 14, &bs);
	printf("add %d\nnew index: %d\n", r, bs);

	r = ldb_close(&db); //close DB
	printf("close result %d\n", r);
    }

    return 0;
}
