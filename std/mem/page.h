#ifndef RK_MEM_PAGE_H
#define RK_MEM_PAGE_H

#include "std/core.h"
#include "std/mem/reg.h"
#include "std/mem/cur.h"

// |================================================================================================|
// |> ALIGN                                                                                         |

/// Returns allocation align in bytes
/// - Linux:   often  4 [KiB]
/// - Windows: often 64 [KiB]
u32 pages_align(void);
/// Returns commit/page align in bytes. Often 4 [KiB]
u32 pages_align_atom(void);

// |================================================================================================|
// |> PAGES                                                                                         |

typedef struct { // TODO: add 2 direction to grow
	union { uptr beg; b64 is_valid; };
	CUR_EMBED(cur, pos, end);
} Pages;

#define PAGES_RAW(_beg, _pos, _end) ((Pages){.beg = (_beg), .pos = (_pos), .end = (_end)})
#define PAGES_NIL PAGES_RAW(0, 0, 0)

// |================================================================================================|
// |> PAGES RESERVE/RELEASE                                                                         |

Pages pages_reserve(u64 min_len);
void  pages_release(Pages * pages);

// |================================================================================================|
// |> PAGES ALLOC                                                                                   |

Pages pages_chop(Pages * ps, u64 min_size, u32 min_align);
uptr  pages_raw(Pages * ps, u64 size, u32 align);

// |================================================================================================|
// |> PAGES PROT                                                                                    |

b8 pages_none(Reg reg);
b8 pages_read(Reg reg);
b8 pages_read_write(Reg reg);
b8 pages_read_exec(Reg reg);

// |================================================================================================|
// |> PAGES API                                                                                     |

// Single user - single pinned constant by max size map
RegMan pages_pinned(Pages * ps);
// Many user - no state, simple wrapper around mmap/VirtualAlloc
RegMan pages_no_state(void);

#endif // !RK_MEM_PAGE_H
