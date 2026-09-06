#include "std/flow/core.h"
#include "std/run/run.h"

void run(Run run) { UNUSED(run);
	Flow_Stat * a1 = FLOW_STAT("a1 {} -> {}");
	Flow_Stat * a2 = FLOW_STAT("a2 {} -> {}");

	flow_enter(a1, FLOW_LOC);
		flow_enter(a2, FLOW_LOC);
	flow_leave(a1);
		PANIC("opsi");
		flow_leave(a2);
};

