#ifndef STD_FILE_H
#define STD_FILE_H

#include "std/mem/reg.h"

typedef struct {
	union { uptr ptr; void * any; u8 * raw; b64 is_valid; };
	u64 len;
} FileBuf;

#define FILE_BUF_RAW ((FileBuf){0})
#define FILE_BUF_NIL ((FileBuf){0})

FileBuf fs_read_all(RegMan man, u8z const * path);
b8      fs_write_all(u8z const * path, void const * ptr, u64 len);

b8 fs_dir_create_single(u8z const * path);
b8 fs_dir_create(u8z * path);

#endif // !STD_FILE_H
