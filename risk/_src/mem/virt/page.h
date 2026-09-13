#ifndef RK_MEM_PAGE_H
#define RK_MEM_PAGE_H

#include "mem/real/stack.h"
#include "mem/quota.h"

// TODO: what about VEH/SEH in Windows and SIGSEGV/ucontext in Linux?

// #define RK_MEM_PROT_DISABLE

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// alloc & commit aligns

usz mem_align_ceil(usz x, usz align);

/// Returns allocation align in bytes.
/// - Linux:   often  4 [KiB];
/// - Windows: often 64 [KiB];
u32 mem_alloc_align(void);

/// Returns commit/page align in bytes. Often 4 [KiB].
u32 mem_commit_align(void);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// virtual memory

typedef struct {
	STACK_EMBED(stack, beg, pos, end);
	Quota * quota;
} Memory;

bool mem_split(Memory * src, Memory * dst, usz min_size, usz min_align);

/// Reserve chunk of virtual memory.
///
/// If failed then:
/// - set [len] to 0;
/// - ret [NULL];
/// Otherwise:
/// - set [len] to ceiled [len] by [mem_alloc_align];
/// - ret valid pointer;
///
/// On [Linux] use [mmap] with:
/// - on [DEBUG]:   [PROT_NONE].
/// - on [RELEASE]: [PROT_READ] and [PROT_WRITE].
///
/// On [Windows] use [VirtualAlloc] with [NOACCESS].
bool mem_reserve(Memory * pages, usz len, Quota * quota);

/// Release memory.
/// If failed then returns [FALSE], otherwise [TRUE].
bool mem_release(void * base, usz len);

/// Commit pages (RW).
///
/// On [Linux] depends on mode do:
/// - [DEBUG]   => mprotect(PROT_READ | PROT_WRITE);
/// - [RELEASE] => nothing;
///
/// On [Windows] use [VirtualAlloc] with [PAGE_READWRITE].
///
/// If failed then returns [FALSE], otherwise [TRUE].
bool mem_commit(void * at, usz len);

/// Commit pages.
///
/// On [Linux] depends on mode do:
/// - [DEBUG]   => mprotect(PROT_EXE | PROT_READ);
/// - [RELEASE] => nothing;
///
/// On [Windows] use [VirtualAlloc] with [PAGE_EXEREAD].
///
/// If failed then returns [FALSE], otherwise [TRUE].
bool mem_set_exe(void * at, usz len);

/// Decommit ceiled page count.
///
/// If failed then returns [FALSE], otherwise [TRUE].
bool mem_decommit(void * base, usz len);

#endif // !RK_MEM_PAGE_H
