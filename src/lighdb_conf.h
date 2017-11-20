#ifndef LIGHDB_CONF_H
#define LIGHDB_CONF_H

//change for your file system library. F.e. for ElmChan's FatFS define LDB_FILE FIL. For STDIO it will be int
#define LDB_FILE int

//Will library be read only
#define LDB_READ_ONLY 0

//Change to 1 if you want use mutexes and change defines below and implement functions
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
