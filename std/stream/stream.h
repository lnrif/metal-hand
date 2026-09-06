#ifndef RK_STREAM_H
#define RK_STREAM_H

#if defined(__linux__) && defined(__x86_64__)
// |================================================================================================|
// |> LINUX (x86_64)                                                                                |
// |================================================================================================|

#include "linux/write.h"

typedef struct { LinuxFd get; } Stream;

#elif defined(_WIN32) && defined(__x86_64__)
// |================================================================================================|
// |> WINDOWS (x86_64)                                                                              |
// |================================================================================================|

#include "os/windows/write.h"

typedef struct { WindowsFd get; } Stream;

#else
	#error "not supported"
#endif

#include "std/str/str.h"

b8 term_enable_utf8(void);

Stream stream_input(void);
Stream stream_output(void);
Stream stream_error(void);

b8 stream_is_terminal(Stream const * stream);
b8 stream_enable_ansi(Stream const * stream);

i64 stream_write(Stream const * stream, void const * ptr, u64 len);

i64 stream_write_str(Stream const * stream, Str s);
#define stream_write_lit(stream, lit) stream_write_str(stream, S(lit))

typedef union {
	Str str;
	struct {
		void const * ptr;
		u64 len;
	};
} StreamVec;

#define SL(lit)   ((StreamVec){.str = S(lit)})
#define SS(_str)  ((StreamVec){.str = (_str)})
#define SZ(str_z) ((StreamVec){.str = str_z_init(str_z).str})

i64 stream_write_vec(Stream const * stream, StreamVec const * ptr, u64 len);

#define stream_write_arr(stream, vecs...) \
	stream_write_vec(stream, (StreamVec[]){vecs}, LEN((StreamVec[]){vecs}))


#endif // !RK_STREAM_H
