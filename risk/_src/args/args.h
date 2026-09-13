#ifndef RK_ARGS_H
#define RK_ARGS_H

#include "trc/assert.h"
#include "fmt/fmt.h"
#include "str/str.h"

////////////////////////////////
// commands

ENUM(ArgsCmdKind, u8) {
	ARGS_CMD_NONE,
	ARGS_CMD_BUILD,
	ARGS_CMD_RUN,
};

typedef struct {
	Str input;
	Str output;
	usz mem;
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
	bool color;
	ArgsCmdKind kind;
} ArgsCmd;

bool args_handle(
	FmtBump * out,
	ArgsCmd * cmd,
	u8 const * const * ptr,
	u32 count
);

////////////////////////////////////////////////////////////////
// internal

////////////////////////////////
// args: state

typedef struct {
	FmtBump * out; StrOpt cmd;
	StrZ compiler, peek;

	u8 const * const * ptr;
	u32 len; u32 at;

	bool color;
	bool help;
} ArgsState;

ArgsState args_state_init(FmtBump * out, u8 const * const * ptr, u32 count);
void args_state_skip(ArgsState * state);
bool args_state_eof(ArgsState const * state);

ENUM(ArgsResult, u8) {
	ARGS_UNKNOWN = FALSE,
	ARGS_KNOWN   = TRUE,
	ARGS_ERROR,
};

#define ARGS_TRY(call) \
	switch (call) { \
		case ARGS_UNKNOWN: break; \
		case ARGS_KNOWN: continue; \
		case ARGS_ERROR: return FALSE; \
	};

ArgsResult args_skip_flag_try(ArgsState * state);

#define args_state_radical(state) args_state_radical_ex(CALL, state)
bool args_state_radical_ex(CallLoc call, ArgsState * state);

////////////////////////////////
// args: memory

typedef struct {
	usz min;
	usz max;
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
bool args_files_finish(ArgsState * state, ArgsFiles * files, bool diag);

#define args_expected_file(state, arg) args_expected_file_ex(CALL, state, arg)
bool args_expected_file_ex(CallLoc call, ArgsState * state, Str arg);

////////////////////////////////
// commands

bool args_help_generic(ArgsState * state);
bool args_build(ArgsState * state, ArgsCmd * cmd);
bool args_help(ArgsState * state);

////////////////////////////////
// common errors

#define args_unknown_command(state) args_unknown_command_ex(CALL, state)
bool args_unknown_command_ex(CallLoc call, ArgsState * state);

#define args_unknown_flag(state) args_unknown_flag_ex(CALL, state)
bool args_unknown_flag_ex(CallLoc call, ArgsState * state);

#define args_unexpected_arg(state) args_unexpected_arg_ex(CALL, state)
bool args_unexpected_arg_ex(CallLoc call, ArgsState * state);

#endif // !RK_ARGS_H
