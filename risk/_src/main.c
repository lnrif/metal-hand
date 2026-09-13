#include "mem/real/bump.h"
#include "sys/sys.h"

int main(void) {
	Bump bump = BUMP_ON_STACK(0x200);
	bump_lit(&bump, "Hello, World!\n");

	sys_write(bump_beg(&bump), bump_len(&bump));
	bump_reset(&bump);

	return 0;
};
