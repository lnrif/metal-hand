#ifndef RK_ARGS_H
#define RK_ARGS_H

#include "std/flow/core.h"
#include "std/fmt/core.h"
#include "std/str/core.h"

////////////////////////////////
// commands

typedef enum: u8 {
	ARGS_CMD_NONE,
	ARGS_CMD_BUILD,
	ARGS_CMD_RUN,
} ArgsCmdKind;

typedef struct {
	Str input;
	Str output;
	u64 mem;
} ArgsCmdBuild;

typedef struct {
	Str input;
	Str output;
} ArgsCmdRun;

typedef struct {
	union {
		ArgsCmdRun run;
		ArgsCmdBuild build;
	} as;
	b8 color;
	ArgsCmdKind kind;
} ArgsCmd;

b8 args_handle(
	Fmt * out,
	ArgsCmd * cmd,
	u8 const * const * ptr,
	u32 count
);

////////////////////////////////////////////////////////////////
// internal

////////////////////////////////
// args: state

typedef struct {
	Fmt * out; StrOpt cmd;
	StrZ compiler, peek;

	u8 const * const * ptr;
	u32 len; u32 at;

	b8 color;
	b8 help;
} ArgsState;

ArgsState args_state_init(Fmt * out, u8 const * const * ptr, u32 count);
void args_state_skip(ArgsState * state);
b8 args_state_eof(ArgsState const * state);

typedef enum: u8 {
	ARGS_UNKNOWN = false,
	ARGS_KNOWN   = true,
	ARGS_ERROR,
} ArgsResult;

#define ARGS_TRY(call) \
	switch (call) { \
		case ARGS_UNKNOWN: break; \
		case ARGS_KNOWN: continue; \
		case ARGS_ERROR: return false; \
	};

ArgsResult args_skip_flag_try(ArgsState * state);

#define args_state_radical(state) args_state_radical_ex(FLOW_LOC, state)
b8 args_state_radical_ex(FlowLoc loc, ArgsState * state);

////////////////////////////////
// args: memory

typedef struct {
	u64 min;
	u64 max;
} ArgsMem;

ArgsResult args_mem_try(ArgsState * state, ArgsMem * mem);

////////////////////////////////
// args: common

ArgsResult args_common_try(ArgsState * state);

////////////////////////////////
// args: files

typedef struct {
	StrOpt input;
	StrOpt output;
} ArgsFiles;

ArgsResult args_files_try(ArgsState * state, ArgsFiles * files);
b8 args_files_finish(ArgsState * state, ArgsFiles * files, b8 diag);

#define args_expected_file(state, arg) args_expected_file_ex(FLOW_LOC, state, arg)
b8 args_expected_file_ex(FlowLoc loc, ArgsState * state, Str arg);

////////////////////////////////
// commands

b8 args_help_generic(ArgsState * state);
b8 args_build(ArgsState * state, ArgsCmd * cmd);
b8 args_help(ArgsState * state);

////////////////////////////////
// common errors

#define args_unknown_command(state) args_unknown_command_ex(FLOW_LOC, state)
b8 args_unknown_command_ex(FlowLoc loc, ArgsState * state);

#define args_unknown_flag(state) args_unknown_flag_ex(FLOW_LOC, state)
b8 args_unknown_flag_ex(FlowLoc loc, ArgsState * state);

#define args_unexpected_arg(state) args_unexpected_arg_ex(FLOW_LOC, state)
b8 args_unexpected_arg_ex(FlowLoc loc, ArgsState * state);

#endif // !RK_ARGS_H
