#include "std/fs/core.h"
#include "linux/write.h"

#if LINUX
	#include "linux/fs.h"
#endif

FileBuf fs_read_all(RegMan man, u8z const * path) {
	#if LINUX
		i64 const size = linux_file_size(path);
		if (size < 0) return FILE_BUF_NIL;

		i64 const open = linux_openat(AT_FDCWD, path, LINUX_FS_READ_ONLY, 0);
		if (open < 0) return FILE_BUF_NIL;
		LinuxFd const fd = (i32)open;

		u64 const alloc_size = (u64)size + 1;
		Reg const mem = reg_upd(man, REG_UPD(REG_NIL, REG_REQ(alloc_size, alignof(u8), REG_DIR_UP)));
		if (!mem.is_valid) {
			linux_close(fd);
			return FILE_BUF_NIL;
		};

		u64 total_read = 0;
		while (total_read < (u64)size) {
			i64 const res = linux_read(fd, mem.raw + total_read, (u64)size - total_read);
			if (res < 0) {
				reg_upd(man, REG_UPD(mem, REG_REQ_FREE));
				linux_close(fd);
				return FILE_BUF_NIL;
			};
			if (res == 0) break;
			total_read += (u64)res;
		};

		mem.raw[total_read] = 0;
		linux_close(fd);

		return (FileBuf){.ptr = mem.ptr, .len = total_read};

	#elif WINDOWS
		#error "TODO: windows"
	#else
		#error "not supported"
	#endif
};

b8 fs_write_all(u8z const * path, void const * ptr, u64 len) {
	#if LINUX
		u32 const flags = LINUX_FS_WRITE_ONLY | LINUX_FS_CREATE | LINUX_FS_TRUNCATE;
		u32 const mode = 0644;
		i64 const open = linux_openat(AT_FDCWD, path, flags, mode);
		if (open < 0) return false;
		LinuxFd const fd = (LinuxFd)open;

		if (len > 0 && ptr != 0) {
			u64 total_written = 0;
			while (total_written < len) {
				i64 const res = linux_write(fd, (u8 const *)ptr + total_written, len - total_written);
				if (res <= 0) {
					linux_close(fd);
					return false;
				};
				total_written += (u64)res;
			};
		};

		linux_close(fd);
		return true;

	#elif WINDOWS
		#error "TODO: windows"
	#else
		#error "not supported"
	#endif
};

b8 fs_dir_create_single(u8z const * path) {
	#if LINUX
		iptr const res = linux_mkdirat(AT_FDCWD, path, 0755);
		if (res == -17) return true; // means exist
		return res >= 0;
	#elif WINDOWS
		#error "TODO: windows"
	#else
		#error "not supported"
	#endif
};

b8 fs_dir_create(u8z * path) {
	#if LINUX
		if (path == 0 || *path == 0) return false;

		for (u64 j = 0; path[j] != 0; j++) {
			if (path[j] == '/' && j > 0) {
				path[j] = 0;
				b8 const ok = fs_dir_create_single((u8z const *)path);
				path[j] = '/';
				if (!ok) return false;
			};
		};

		return fs_dir_create_single((u8z const *)path);
	#elif WINDOWS
		#error "TODO: windows"
	#else
		#error "not supported"
	#endif
};

