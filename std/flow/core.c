#include "std/flow/core.h"
#include "std/proc/exit.h"
#include "std/stream/core.h"

// |================================================================================================|
// |> panic                                                                                         |

void NO_RETURN flow_panic(FLowPanic panic) {
	Stream err = stream_output();

	stream_write_arr(&err,
		SL("\n" "\x1B[1;31m" "[panic] " "\x1B[1;37m"), SS(panic.msg), SL("\n"),
		SL("\x1B[1;36m" " --> "), SZ(panic.loc.get), SL("\n"),
		SL("\x1B[1;36m" " --> "), SS(S(FLOW_LOC_LIT)), SL("\n\n"),
	);

	proc_exit(1);
};

