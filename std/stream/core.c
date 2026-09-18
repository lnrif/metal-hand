#include "std/stream/core.h"

#if LINUX
// |================================================================================================|
// |> LINUX (x86_64)                                                                                |
// |================================================================================================|

b8 term_enable_utf8(void) { return true; };

Stream stream_input (void) { return (Stream){.get = LINUX_INPUT_FD }; };
Stream stream_output(void) { return (Stream){.get = LINUX_OUTPUT_FD}; };
Stream stream_error (void) { return (Stream){.get = LINUX_ERROR_FD }; };

b8 stream_is_terminal(Stream const * stream) { return linux_is_tty(stream->get); };
b8 stream_enable_ansi(Stream const * stream) { UNUSED(stream); return true; };

i64 stream_write(Stream const * stream, void const * ptr, u64 len) {
	uptr const beg = (uptr)ptr;
	uptr const end = beg + len;

	uptr pos = beg;
	while (pos < end) {
		i64 const ret = linux_write(stream->get, (void*)pos, end - pos);
		if ((u64)-4095 <= (u64)ret) return -1;
		if (ret == 0) break;
		pos += (u64)ret;
	};

	return (i64)(pos - beg);
};

i64 stream_write_str(Stream const * stream, Str s) {
	return stream_write(stream, s.raw, s.len);
};

i64 stream_write_vec(Stream const * stream, StreamVec const * ptr, u64 len) {
	return linux_write_vec(stream->get, (LinuxVec *)ptr, (u32)len);
};

#elif WINDOWS
// |================================================================================================|
// |> WINDOWS (x86_64)                                                                              |
// |================================================================================================|

// KERNEL32.DLL
IMPORT b32 GetConsoleMode(void * hConsoleHandle, u32 * lpMode);
IMPORT b32 SetConsoleMode(void * hConsoleHandle, u32 dwMode);
IMPORT b32 SetConsoleOutputCP(unsigned int wCodePageID);

#include "os/windows/write.h"

b8 term_enable_utf8(void) {
	return (b8)SetConsoleOutputCP(65001);
};

Stream stream_input (void) { return (Stream){.get = GetStdHandle(WINDOWS_INPUT_HANDLE)  }; };
Stream stream_output(void) { return (Stream){.get = GetStdHandle(WINDOWS_OUTPUT_HANDLE) }; };
Stream stream_error (void) { return (Stream){.get = GetStdHandle(WINDOWS_ERROR_HANDLE)  }; };

b8 stream_is_terminal(Stream const * stream) {
	u32 mode;
	return (b8)GetConsoleMode(stream->get, &mode);
};

#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004

b8 stream_enable_ansi(Stream const * stream) {
	// take stream mode
	u32 mode = 0;
	if (!GetConsoleMode(stream->get, &mode)) return false;
	// apply ansi
	mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	return (b8)SetConsoleMode(stream->get, mode);
};

i64 stream_write(Stream const * stream, Str s) {
	uptr const beg = (uptr)s.ptr;
	uptr const end = beg + s.len;

	uptr ptr = beg;
	while (ptr < end) {
		u32 const len = (u32)MIN(end - ptr, GB(2));

		u32 written;
		if (!WriteFile(stream->get, (void*)ptr, len, &written, 0)) return -1;
		if (written == 0) break;

		ptr += written;
	};

	return (i64)(ptr - beg);
};

i64 stream_write_vec(Stream const * stream, StreamVec const * ptr, u64 len) {
	for (u64 i = 0; i < len; i += 1) {
		Str const str = ptr[i].str;
		if (stream_write(stream, str) != (i64)str.len) return -1;
	};
	return 0;
};

#endif

