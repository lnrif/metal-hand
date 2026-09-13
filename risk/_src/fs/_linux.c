#include "fs/fs.h"
#include "mem/real/bump.h"

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

////////////////////////////////
// atoms

FsAtom fs_atom_peek_front(FsAtoms const * atoms);

FsAtom fs_atom_peek_back(FsAtoms const * atoms);

FsAtom fs_atom_next_front(FsAtoms * atoms);

FsAtom fs_atom_next_back(FsAtoms * atoms);

////////////////////////////////
// extention

FsExt fs_atom_extention(FsAtoms const * atoms);

FsExt fs_ext_peek(FsExt * ext);

FsExt fs_ext_next(FsExt * ext);

////////////////////////////////
// path

// ...

////////////////////////////////
// file

FsFileOpen fs_file_open(FsFile * file, FsPathZ path, int fd) {
	if (fd < 0) {
		if      (errno == ENOENT) return FS_FILE_OPEN_NOT_FOUND;
		else if (errno == EACCES) return FS_FILE_OPEN_DENIED;
		else                      return FS_FILE_OPEN_UNKNOWN;
	};

	struct stat sb;
	if (fstat(fd, &sb) < 0)   { close(fd); return FS_FILE_OPEN_UNKNOWN;  };
	if (!S_ISREG(sb.st_mode)) { close(fd); return FS_FILE_OPEN_NOT_FILE; };

	*file = (FsFile){
		.path = path,
		.fd = (u32)fd,
		.size = (usz)sb.st_size,
	};

	return FS_FILE_OPEN_OK;
};

FsFileOpen fs_file_open_for_read(FsFile * file, FsPathZ path) {
	ASSERT_DEBUG(file != NULL, "expected [file]");
	int fd = open((const char*)path.ptr_z, O_RDONLY);
	return fs_file_open(file, path, fd);
};

FsFileOpen fs_file_create_blank(FsFile * file, FsPathZ path) {
	ASSERT_DEBUG(file != NULL, "expected [file]");
	int fd = open((const char*)path.ptr_z, O_RDWR | O_CREAT | O_TRUNC | O_APPEND, 0664);
	return fs_file_open(file, path, fd);
};

FsFileOpen fs_file_create_exec(FsFile * file, FsPathZ path) {
	ASSERT_DEBUG(file != NULL, "expected [file]");
	int fd = open((const char*)path.ptr_z, O_RDWR | O_CREAT | O_TRUNC | O_APPEND, 0775);
	return fs_file_open(file, path, fd);
};

FsFileLoad fs_file_load(FsFile const * file, Bump * bump) {
	ASSERT_DEBUG(file != NULL, "expected [file]");
	ASSERT_DEBUG(file->fd != 0, "expected [file.fd]");

	usz const len = file->size;
	uptr const mark = bump_mark(bump);
	u8 * const ptr = bump_array(bump, len, u8);
	if (ptr == NULL) return FS_FILE_LOAD_OOM;

	if (lseek((int)file->fd, 0, SEEK_SET) == -1) goto error;

	for (usz i = 0; i < file->size;) {
		isz n = read((int)file->fd, ptr + i, len - i);

		if (n > 0) {
			i += (usz)n;
			continue;
		};

		if (n < 0 && errno != EINTR) goto error;
	};

	return FS_FILE_LOAD_OK;

error:
	bump_load(bump, mark);
	return FS_FILE_LOAD_UNKNOWN;
};

bool fs_file_write(FsFile * file, Str str) {
	ASSERT_DEBUG(file != NULL, "expected [file]");

	for (usz i = 0; i < str.len;) {
		isz n = write((int)file->fd, str.ptr + i, str.len - i);

		if (n > 0) {
			i += (usz)n;
			file->size += (usz)n;
			continue;
		};

		if (n < 0 && errno != EINTR) return FALSE;
	}

	return TRUE;
};

void fs_file_close(FsFile * file) {
	if (file == NULL || file->fd == 0) return;
	close((int)file->fd);
	file->fd = 0;
	file->size = 0;
};

////////////////////////////////
// directory

FsDirOpen fs_dir_open(FsDir * dir, FsPathZ path) {
	ASSERT_DEBUG(dir != NULL, "expected [dir]");
	int fd = open((const char*)path.ptr_z, O_RDONLY | O_DIRECTORY);

	if (fd < 0) {
		if      (errno == ENOENT)       return FS_DIR_OPEN_NOT_FOUND;
		else if (errno == EACCES)       return FS_DIR_OPEN_DENIED;
		else if (errno == ENOTDIR)      return FS_DIR_OPEN_NOT_DIR;
		else if (errno == ENAMETOOLONG) return FS_DIR_OPEN_NAME_TOO_LONG;
		else                            return FS_DIR_OPEN_UNKNOWN;
	}

	struct stat sb;
	if (fstat(fd, &sb) < 0)   { close(fd); return FS_DIR_OPEN_UNKNOWN; };
	if (!S_ISDIR(sb.st_mode)) { close(fd); return FS_DIR_OPEN_NOT_DIR; };

	*dir = (FsDir){.path = path, .fd = (usz)fd};
	return FS_DIR_OPEN_OK;
};

void fs_dir_close(FsDir * dir) {
	if (dir == NULL || dir->fd == 0) return;
	close((int)dir->fd);
	dir->fd = 0;
};

FsDirCreate fs_dir_create(FsPathZ path, Bump * tmp) {
	FsDirCreate err = FS_DIR_CREATE_UNKNOWN;
	if (path.len == 0) return FS_DIR_CREATE_EMPTY_PATH;

	uptr const mark = bump_mark(tmp);
	u8 * const buf = bump_array(tmp, path.len + 1, u8);
	if (buf == NULL) return FS_DIR_CREATE_OOM;

	for (usz i = 0; i <= path.len; i += 1) { buf[i] = path.ptr_z[i]; };

	usz i = buf[0] == '/' ? 1 : 0;
	for (; i < path.len; i += 1) {
		if (buf[i] != '/') continue;

		buf[i] = '\0';
		if (mkdir((char*)buf, 0775) < 0 && errno != EEXIST) goto error;
		buf[i] = '/';
	};

	if (mkdir((char*)buf, 0775) < 0 && errno != EEXIST) goto error;

	bump_load(tmp, mark);
	return FS_DIR_CREATE_OK;

error:
	if      (errno == EACCES)       err = FS_DIR_CREATE_DENIED;
	else if (errno == ENOTDIR)      err = FS_DIR_CREATE_NOT_DIR;
	else if (errno == ENOSPC)       err = FS_DIR_CREATE_NO_SPACE;
	else if (errno == EROFS)        err = FS_DIR_CREATE_RO_FS;
	else if (errno == ENAMETOOLONG) err = FS_DIR_CREATE_NAME_TOO_LONG;
	else if (errno == ELOOP)        err = FS_DIR_CREATE_LOOP;

	bump_load(tmp, mark);
	return err;
};

FsFileOpen fs_file_open_for_read_at(FsDir const * dir, FsFile * file, FsPathZ path) {
	ASSERT_DEBUG(dir != NULL, "expected [dir]");
	ASSERT_DEBUG(dir->fd != 0, "expected [dir.fd]");
	int fd = openat((int)dir->fd, (const char*)path.ptr_z, O_RDONLY);
	return fs_file_open(file, path, fd);
};

FsFileOpen fs_file_create_blank_at(FsDir const * dir, FsFile * file, FsPathZ path) {
	ASSERT_DEBUG(dir != NULL, "expected [dir]");
	ASSERT_DEBUG(dir->fd != 0, "expected [dir.fd]");
	int fd = openat((int)dir->fd, (const char*)path.ptr_z, O_RDWR | O_CREAT | O_TRUNC, 0644);
	return fs_file_open(file, path, fd);
};

FsFileOpen fs_file_create_exec_at(FsDir const * dir, FsFile * file, FsPathZ path) {
	ASSERT_DEBUG(dir != NULL, "expected [dir]");
	ASSERT_DEBUG(dir->fd != 0, "expected [dir.fd]");
	int fd = openat((int)dir->fd, (const char*)path.ptr_z, O_RDWR | O_CREAT | O_TRUNC, 0755);
	return fs_file_open(file, path, fd);
};

