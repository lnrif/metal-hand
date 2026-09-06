#ifndef STD_RUN_AUX_H
#define STD_RUN_AUX_H

#include "std/run/run.h"

typedef enum {
	AUX_NULL          = 0,
	AUX_IGNORE        = 1,
	AUX_EXECFD        = 2,
	AUX_PHDR          = 3,
	AUX_PHENT         = 4,
	AUX_PHNUM         = 5,
	AUX_PAGESZ        = 6,
	AUX_BASE          = 7,
	AUX_FLAGS         = 8,
	AUX_ENTRY         = 9,
	AUX_NOTELF        = 10,
	AUX_UID           = 11,
	AUX_EUID          = 12,
	AUX_GID           = 13,
	AUX_EGID          = 14,
	AUX_PLATFORM      = 15,
	AUX_HWCAP         = 16,
	AUX_CLKTCK        = 17,
	AUX_SECURE        = 23,
	AUX_BASE_PLATFORM = 24,
	AUX_RANDOM        = 25,
	AUX_HWCAP2        = 26,
	AUX_EXECFN        = 31,
	AUX_SYSINFO_EHDR  = 33,
} AuxType;

typedef struct {
	u64 type;
	u64 expr;
} Aux;

#define AUX_NONE ((Aux){.type = U64_MAX, .expr = U64_MAX})
#define AUX_IS_NONE(aux) ((aux)->type == U64_MAX)

#if defined(__linux__)
// |================================================================================================|
// |> LINUX                                                                                         |
// |================================================================================================|

typedef struct { u64 const * at; } AuxIter;

AuxIter aux_init(Run const * run);
b8      aux_eof(AuxIter const * iter);
Aux     aux_peek(AuxIter const * iter);
Aux     aux_next(AuxIter * iter);

#else
	#error "this file expected to use only in linux"
#endif

#endif // !STD_RUN_AUX_H
