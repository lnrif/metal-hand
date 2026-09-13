#ifndef RK_FS_H
#define RK_FS_H

#include "str/str.h"
#include "mem/mem.h"

////////////////////////////////
// atoms

// Slice of [FsPathZ], NOT HAVE NULL-TERMINATOR.
//
//   vvvv    vvvv
// "/usr/bin/fish"       -> ["/", "usr/", "bin/", "fish"]
//  ^    ^^^^
//
// "///usr///bin///fish" -> ["/", "usr/", "bin/", "fish"]
typedef struct {
	STR_EMBED(str, ptr, len);
} FsAtoms;

// Atom of path: file or directory name
//
//   vvvv    vvvv
// "/usr/bin/fish" -> ["/", "usr/", "bin/", "fish"] <- atoms
//  ^    ^^^^
typedef union {
	STR_EMBED(str, ptr, len);
	FsAtoms atoms;
} FsAtom;

// Peek front atom and returns it.
// If there are not atoms returns empty atom (len == 0)
//
// "~/.config/nvim/init.lua" -> "~/"
//  ^^
FsAtom fs_atom_peek_front(FsAtoms const * atoms);

// Peek back atom and returns it.
// If there are not atoms returns empty atom (len == 0)
//
// "~/.config/nvim/init.lua" -> "init.lua"
//                 ^^^^^^^^
FsAtom fs_atom_peek_back(FsAtoms const * atoms);

// Take next front atom and returns it.
// If there are not atoms returns empty atom (len == 0)
//
// "~/.config/nvim/init.lua" -> (atom: "~/", atoms: ".config/nvim/init.lua")
FsAtom fs_atom_next_front(FsAtoms * atoms);

// Take next back atom and returns it.
// If there are not atoms returns empty atom (len == 0)
//
// "~/.config/nvim/init.lua" -> (atoms: "~/.config/nvim/", atom: "init.lua")
FsAtom fs_atom_next_back(FsAtoms * atoms);

////////////////////////////////
// extention

// File extention.
typedef struct {
	STR_EMBED(str, ptr, len);
} FsExt;

// Returns extention of back atom.
// If there are not extention returns empty string (len == 0)
//
// "~/.config/nvim/init.lua"         -> "lua"
//                     ^^^^
// "~/.config/nvim/init.lua.lua.lua" -> "lua.lua.lua"
//                     ^^^^^^^^^^^^
FsExt fs_atom_extention(FsAtoms const * atoms);

// Peek part of extention and returns it.
//
// "lua"        -> "lua"
// "lua.z7.git" -> "git"
FsExt fs_ext_peek(FsExt * ext);

// Take part of extention and returns it.
//
// "lua"        -> (ext:       "", next: "lua")
// "lua.z7.git" -> (ext" "lua.z7", next: "git")
FsExt fs_ext_next(FsExt * ext);

////////////////////////////////
// path

// Null-terminated path.
typedef union {
	// ptr_z[len] == '\0'
	STR_EMBED(str_z, ptr_z, len);
} FsPathZ;

#define FS_PATH(lit) ((FsPathZ){.str_z = STR(lit)})
// [FsPathZ] path
#define FS_ATOMS(path) ((FsAtoms){.str = (path).str_z})

////////////////////////////////
// file

typedef struct {
	FsPathZ path; usz size;
	u32 fd;
} FsFile;

// ENUM(FsFileMode, u8) {
// 	// 3 base
// 	FS_FILE_READ  = 0b001,
// 	FS_FILE_WRITE = 0b010,
// 	FS_FILE_EXEC  = 0b100,
// 	// shortcuts
// 	FS_FILE_RW    = FS_FILE_READ | FS_FILE_WRITE,
// 	FS_FILE_RE    = FS_FILE_READ | FS_FILE_EXEC,
// };

ENUM(FsFileOpen, u8) {
	FS_FILE_OPEN_OK,
	FS_FILE_OPEN_NOT_FOUND,
	FS_FILE_OPEN_NOT_FILE,
	FS_FILE_OPEN_DENIED,
	FS_FILE_OPEN_NAME_TOO_LONG,
	FS_FILE_OPEN_UNKNOWN,
};

// Open (not creates) file only for reading.
MUST_USE FsFileOpen fs_file_open_for_read(FsFile * file, FsPathZ path);

// Creates blank file for reading and writing.
// Sets mode to 'rw-rw-r--' or in octal 0664.
// Creates directories if not exist.
MUST_USE FsFileOpen fs_file_create_blank(FsFile * file, FsPathZ path);

// Creates blank file for reading and writing.
// Sets mode to 'rwxrwxr-x' or in octal 0775.
// Creates directories if not exist.
MUST_USE FsFileOpen fs_file_create_exec(FsFile * file, FsPathZ path);

ENUM(FsFileLoad, u8) {
	FS_FILE_LOAD_OK,
	// [advice]: you can easily check [file.size] and allocate enough memory!
	FS_FILE_LOAD_OOM,
	FS_FILE_LOAD_UNKNOWN,
};

// Append file content to bump.
// [!] Not support virtual files.
MUST_USE FsFileLoad fs_file_load(FsFile const * file, Bump * bump);

// Write string in file.
MUST_USE bool fs_file_write(FsFile * file, Str str);

// Closes file.
void fs_file_close(FsFile * file);

////////////////////////////////
// directory

typedef struct {
	FsPathZ path;
	usz fd;
} FsDir;

ENUM(FsDirOpen, u8) {
	FS_DIR_OPEN_OK,
	FS_DIR_OPEN_DENIED,
	FS_DIR_OPEN_NOT_FOUND,
	FS_DIR_OPEN_NOT_DIR,
	FS_DIR_OPEN_NAME_TOO_LONG,
	FS_DIR_OPEN_UNKNOWN,
};

FsDirOpen fs_dir_open(FsDir * dir, FsPathZ path);
void fs_dir_close(FsDir * dir);

ENUM(FsDirCreate, u8) {
	FS_DIR_CREATE_OK,
	FS_DIR_CREATE_OOM,
	FS_DIR_CREATE_EMPTY_PATH,
	FS_DIR_CREATE_DENIED,
	FS_DIR_CREATE_NOT_DIR,
	FS_DIR_CREATE_NO_SPACE,
	FS_DIR_CREATE_RO_FS,
	FS_DIR_CREATE_NAME_TOO_LONG,
	FS_DIR_CREATE_LOOP,
	FS_DIR_CREATE_UNKNOWN,
};

// Creates directories with mode '0755' if not exist.
//
// Uses temporary bump for path building.
MUST_USE FsDirCreate fs_dir_create(FsPathZ path, Bump * tmp);

FsFileOpen fs_file_open_for_read_at(FsDir const * dir, FsFile * file, FsPathZ path);
FsFileOpen fs_file_create_blank_at(FsDir const * dir, FsFile * file, FsPathZ path);
FsFileOpen fs_file_create_exec_at(FsDir const * dir, FsFile * file, FsPathZ path);

#endif // !RK_FS_H
