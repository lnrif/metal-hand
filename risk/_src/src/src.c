#include "src.h"

SrcDot src_eval_dot(Str src, usz at) {
	ASSERT_DEBUG(str_sane(src), "invalid invariant");
	src = str_sub(src, 0, at + 1);

	SrcDot dot = {.row = 1, .col = 0};
	for (usz i = 0; i < src.len; i += 1) {
		if (src.ptr[i] == '\n') {
			dot.row += 1;
			dot.col = 0;
		} else {
			dot.col += 1;
		};
	};

	if (dot.col == 0) dot.col = 1;
	return dot;
};

static void src_load_fmt(CallLoc call, FmtVirt * out, FsFileOpen result, FsPathZ path) {
	if (result == FS_FILE_OPEN_OK) return;
	switch (result) {
		case FS_FILE_OPEN_NOT_FILE: fmt_virt_write(out,
			FMT_COLOR(FMT_RED), FMT_LIT("[err:src] "),
			FMT_COLOR(FMT_WHITE), FMT_LIT("expected file, but found folder "),
			FMT_COLOR(FMT_RED), FMT_LIT("'"), FMT_STR(path.str_z), FMT_LIT("'"), FMT_LIT("\n"),
		); break;
		case FS_FILE_OPEN_NOT_FOUND: fmt_virt_write(out,
			FMT_COLOR(FMT_RED), FMT_LIT("[err:src] "),
			FMT_COLOR(FMT_WHITE), FMT_LIT("cannot found file "),
			FMT_COLOR(FMT_RED), FMT_LIT("'"), FMT_STR(path.str_z), FMT_LIT("'"), FMT_LIT("\n"),
		); break;
		case FS_FILE_OPEN_DENIED: fmt_virt_write(out,
			FMT_COLOR(FMT_RED), FMT_LIT("[err:src] "),
			FMT_COLOR(FMT_WHITE), FMT_LIT("access denied for file "),
			FMT_COLOR(FMT_RED), FMT_LIT("'"), FMT_STR(path.str_z), FMT_LIT("'"), FMT_LIT("\n"),
		); break;
		default: fmt_virt_write(out,
			FMT_COLOR(FMT_RED), FMT_LIT("[err:src] "),
			FMT_COLOR(FMT_WHITE), FMT_LIT("failed open file "),
			FMT_COLOR(FMT_RED), FMT_LIT("'"), FMT_STR(path.str_z), FMT_LIT("'"), FMT_LIT("\n"),
		); break;
	};

	fmt_virt_write(out,
		FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_LOC_DEBUG(call),
		FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_LOC_DEBUG(CALL),
		FMT_COLOR(FMT_RESET),
	);
};

static void src_load_fail_to_copy(FmtVirt * out, FsPathZ path) {
	fmt_virt_write(out,
		FMT_COLOR(FMT_RED), FMT_LIT("[err:src] "),
		FMT_COLOR(FMT_WHITE), FMT_LIT("failed to copy file "),
		FMT_COLOR(FMT_RED), FMT_LIT("'"), FMT_STR(path.str_z), FMT_LIT("'"), FMT_LIT("\n"),
		FMT_COLOR(FMT_RED), FMT_LIT("'\n"),
		FMT_COLOR(FMT_RED), FMT_LIT("| "), FMT_LOC_DEBUG(CALL),
		FMT_COLOR(FMT_RESET),
	);
};

bool src_load_ex(CallLoc call, FmtVirt * out, VirtBump * bump, Src * src, FsPathZ path) {
	FsFile file;

	FsFileOpen result = fs_file_open_for_read(&file, path);
	if (result != FS_FILE_OPEN_OK) {
		src_load_fmt(call, out, result, path);
		return FALSE;
	};

	u8 * ptr = virt_bump_array(bump, file.size + 1, u8);
	if (ptr == NULL || !virt_bump_commit(bump)) goto error;
	Bump tmp = bump_from_slice(ptr, file.size);
	if (fs_file_load(&file, &tmp) != FS_FILE_LOAD_OK) goto error;
	ptr[file.size] = '\0';

	*src = (Src){.path = path, .ptr_z = ptr, .len = file.size};

	fmt_virt_write(out,
		FMT_COLOR(FMT_BLUE), FMT_LIT("[info] "),
		FMT_COLOR(FMT_WHITE), FMT_LIT("loaded file "),
		FMT_COLOR(FMT_BLUE), FMT_LIT("'"), FMT_STR(path.str_z), FMT_LIT("'"),
		FMT_COLOR(FMT_PURPLE), FMT_LIT(" (size: "), FMT_MEM(file.size), FMT_LIT(")"),
		FMT_LIT(" "), FMT_LOC_DEBUG(CALL),
		FMT_COLOR(FMT_RESET),
	);

	fs_file_close(&file);
	return TRUE;

error:
	src_load_fail_to_copy(out, path);
	fs_file_close(&file);
	return FALSE;
};
