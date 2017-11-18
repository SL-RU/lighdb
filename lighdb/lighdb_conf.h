#ifndef LIGHDB_CONF_H
#define LIGHDB_CONF_H

//USE_STDIO == 1 enables STDIO implementation of ldb_io_* so you don't need to do it in your code. If == 0 then can implement that functions by yourslef 
#define LIGHDB_USE_STDIO 1
#define LDB_FILE

#define LDB_MUTEX 0

#if LDB_MUTEX == 1
#include "FreeRTOS.h"
#include "semphr.h"
#define LDB_MUTEX_t xSemaphoreHandle //change for your OS
//implement that functions for your OS

//create mutex object
uint8_t ldb_mutex_create (LDB_MUTEX_t *sobj);
//delete mutex
uint8_t ldb_mutex_delete (LDB_MUTEX_t *sobj);
//Request Grant to Access some object
uint8_t ldb_mutex_request_grant (LDB_MUTEX_t *sobj);
//Release Grant to Access the Volume
uint8_t ldb_mutex_release_grant (LDB_MUTEX_t *sobj);
#endif

#endif
