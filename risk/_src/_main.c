#include "fmt/fmt.h"
#include "fs/fs.h"
#include "mem/virt.h"
#include "prs/ast.h"
#include "prs/parser.h"
#include "src/src.h"
#include "args/args.h"

int main(int argc, char ** argv) {
	// parse args
	// CmdCommand cmd; {
	// 	Memory mem; usz const out_size = MB(1);
	// 	ASSERT(mem_reserve(&mem, MB(out_size), NULL), "");
	//
	// 	FmtVirt out = (FmtVirt){
	// 		.virt = virt_bump_init(&mem, .size = out_size),
	// 		.color = TRUE,
	// 	};
	//
	// 	bool const ok = cmd_handle_args(&out, &cmd, argc, argv);
	// 	fmt_virt_flush(&out);
	//
	// 	mem_release((void*)mem.beg, mem.end - mem.beg);
	// 	if (!ok) exit(1);
	// };

	// nothing do
	// if (cmd.kind == CMD_COMMAND_NONE) exit(0);

	// quota_print(mem.quota, &QUOTA_THEME_DEFAULT, STR("usage"));

	u8 OUT[KB(4)]; FmtBump out = (FmtBump){
		.bump = bump_from_array(OUT),
		.color = TRUE,
	};

	ArgsCmd cmd = {0}; {
		if (!args_handle(&out, &cmd, (u8 const * const *)argv, (u32)argc)) goto err;
		fmt_flush(&out);
	};

	if (cmd.kind == ARGS_CMD_NONE) goto ok;
	if (cmd.kind == ARGS_CMD_BUILD) {
		u8 PATH[KB(4)]; FmtBump path_bump = (FmtBump){
			.bump = bump_from_array(PATH),
			.color = FALSE,
		};

		FsPathZ const input     = {fmt_write(&path_bump, FMT_STR(cmd.as.build.input),  FMT_LIT("\0"))};
		FsPathZ const ast_graph = {fmt_write(&path_bump, FMT_STR(cmd.as.build.output), FMT_LIT(".dot"), FMT_LIT("\0"))};
		// FsPathZ const output    = {fmt_write(&path_bump, FMT_STR(cmd.as.build.output), FMT_LIT("\0"))};
		// FsPathZ const debug_asm = {fmt_write(&path_bump, FMT_STR(cmd.as.build.output), FMT_LIT(".asm"), FMT_LIT("\0"))};

		// fmt_write(&out,
		// 	FMT_COLOR(FMT_BLUE), FMT_LIT("[info] "),
		// 	FMT_COLOR(FMT_WHITE), FMT_LIT("input:  "),
		// 	FMT_COLOR(FMT_CYAN), FMT_LIT("\""), FMT_STR(input.str_z), FMT_LIT("\""),
		// 	FMT_LIT("\n       "),
		// 	FMT_COLOR(FMT_WHITE), FMT_LIT("output: "),
		// 	FMT_COLOR(FMT_CYAN), FMT_LIT("\""), FMT_STR(output.str_z), FMT_LIT("\" "),
		// 	FMT_LOC_DEBUG(CALL),
		// 	// FMT_COLOR(FMT_WHITE), FMT_LIT("asm:    "),
		// 	// FMT_COLOR(FMT_CYAN), FMT_LIT("\""), FMT_STR(debug_asm.str_z), FMT_LIT("\""), FMT_LIT("\n"),
		// );
		// fmt_flush(&out);

		Memory mem; Quota quota = quota_init(cmd.as.build.mem, NULL);
		ASSERT(mem_reserve(&mem, GB(16), &quota), "");

		FmtVirt build = (FmtVirt){
			.virt = virt_bump_init(&mem, .size = KB(16)),
			.color = cmd.color,
		};

		VirtBump src_bump = virt_bump_init(&mem, .size = KB(256));

		Src src;
		if (!src_load(&build, &src_bump, &src, input)) {
			fmt_flush(&build);
			goto err;
		};

		Lex lex = lex_analyze(&mem, &build, src);
		fmt_flush(&build);

		Parser parser = parser_init(&mem, &build, lex);
		AstIdx expr = parser_root(&parser);
		// Stack const * stack = &parser.raw.pool.stack;
		// fmt_write(&build, FMT_MEM(stack->end - stack->pos), FMT_LINE);
		// ast_fmt(&build, &parser.raw, expr);

		{
			uptr const mark = mem.pos;

			FmtVirt tmp = (FmtVirt){
				.virt = virt_bump_init(&mem, .size = MB(16)),
				.color = FALSE,
			};

			ast_graphviz(&tmp, &parser.raw, expr);
			FsFile file; FsFileOpen code = fs_file_create_blank(&file, ast_graph);
			ASSERT(code == FS_FILE_OPEN_OK, "filled to create file '%s'", ast_graph.ptr_z);

			ASSERT(fs_file_write(&file, fmt_as_str(&tmp)), "failed to write");
			fs_file_close(&file);
			virt_bump_decommit(&tmp.virt);

			mem.pos = mark;
		};

		fmt_flush(&build);

		ASSERT_DEBUG(mem_release((void*)mem.beg, mem.end - mem.beg), "");
		goto ok;
	};

	if (cmd.kind == ARGS_CMD_RUN) {
		fmt_write(&out, FMT_COLOR(FMT_RED), FMT_LIT("[todo] "), FMT_COLOR(FMT_WHITE), FMT_LIT("'run' command not implemented"), FMT_LIT("\n"));
		goto err;
	};

ok:
	fmt_flush(&out);
	exit(0);

err:
	fmt_flush(&out);
	exit(1);
};

/*
		u8 TMP[KB(4)]; FmtBump tmp = (FmtBump){
			.bump = bump_from_array(TMP),
			.color = FALSE,
		};

		u32 const need_len = fmt_bump_str(&tmp, STR(
			"format ELF64 executable 3" "\n"
			"entry start" "\n"
			"\n"

			"segment readable executable" "\n"
			"start:" "\n"
			"\t" "; write(...)" "\n"
			"\t" "mov rax, 1" "\n"
			"\t" "mov rdi, 1" "\n"
			"\t" "mov rsi, msg" "\n"
			"\t" "mov rdx, len" "\n"
			"\t" "syscall" "\n"
			"\t" "; exit(0)" "\n"
			"\t" "mov rax, 60" "\n"
			"\t" "mov rdi, 0" "\n"
			"\t" "syscall" "\n"
			"\n"

			"segment readable" "\n"
			"; message" "\n"
			"msg: db \"Hello, World!\", 10" "\n"
			"len = $ - msg" "\n"
			"\n"
		), (FmtOpt){0}, NULL);
		ASSERT(need_len == 0, "");

		{
			FsFile file; FsFileOpen code = fs_file_create_blank(&file, debug_asm);
			ASSERT(code == FS_FILE_OPEN_OK, "filled to create file '%s'", debug_asm.ptr_z);

			ASSERT(fs_file_write(&file, fmt_as_str(&tmp)), "failed to write");
			fs_file_close(&file);
		};
*/
