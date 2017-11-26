#Varibles:
#  LDB_IMPLEMENTATIONS_STDIO. Set 1 to use lighdb_stdio.c
#  LDB_IMPLEMENTATIONS_FATFS. Set 1 to use lighdb_fatfs.c
#  LDB_IMPLEMENTATIONS_FREERTOS. Set 1 to use lighdb_freertos.c

set(srcs "src/lighdb.c")
if(${LDB_IMPLEMENTATIONS_STDIO})
  set(srcs ${srcs} "implementations/lighdb_stdio.c")
endif(${LDB_IMPLEMENTATIONS_STDIO})
if(${LDB_IMPLEMENTATIONS_FATFS})
  set(srcs ${srcs} "implementations/lighdb_fatfs.c")
endif(${LDB_IMPLEMENTATIONS_FATFS})
if(${LDB_IMPLEMENTATIONS_FREERTOS})
  set(srcs ${srcs} "implementations/lighdb_freertos.c")
endif(${LDB_IMPLEMENTATIONS_FREERTOS})

message("${srcs}")

add_library(lighdb ${srcs})
target_include_directories(lighdb PUBLIC src)
