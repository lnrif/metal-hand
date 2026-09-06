#if defined(__linux__)
// |================================================================================================|
// |> LINUX                                                                                         |
// |================================================================================================|

#include "std/run/aux.h"

AuxIter aux_init(Run const * run) {
	u8 const * const * at = run->env;
	while (*at != 0) at += 1;
	return (AuxIter){.at = (void*)at};
};

b8 aux_eof(AuxIter const * iter) {
	return *iter->at == 0;
};

Aux aux_peek(AuxIter const * iter) {
	if (*iter->at == 0) return AUX_NONE;
	return *(Aux*)iter->at;
};

Aux aux_next(AuxIter * iter) {
	if (*iter->at == 0) return AUX_NONE;
	Aux peek = *(Aux*)iter->at;
	iter->at += sizeof(Aux) / sizeof(u64);
	return peek;
};

#endif

