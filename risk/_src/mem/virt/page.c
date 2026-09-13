#include "mem/virt/page.h"
#include "assert.h"
#include "core/types.h"
#include "mem/quota.h"

// #define MEM_PROT_DISABLE

usz mem_align_ceil(usz x, usz align) {
	ASSERT_DEBUG(align != 0 && ((align & (align - 1)) == 0), "invalid [align]");
	bool const overflow = ADD_OVER_DBG(x, align - 1, &x);
	ASSERT_DEBUG(!overflow, "overflow");
	return x & ~(align - 1);
};

#ifdef __linux__
	#include <unistd.h>
	#include <sys/mman.h>
#elif _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#else
	#error "not supported platfrom"
#endif

#ifdef __linux__
	static u32 MEM_ALIGN_UNWRAP = 0;
	
	static void mem_init(void) {
		MEM_ALIGN_UNWRAP = (u32)sysconf(_SC_PAGESIZE);
	};
	
	u32 mem_alloc_align(void) {
		if (MEM_ALIGN_UNWRAP == 0) mem_init();
		return MEM_ALIGN_UNWRAP;
	};
	
	u32 mem_commit_align(void) {
		return mem_alloc_align();
	};
#elif _WIN32
	static u32 MEM_ALLOC_ALIGN_UNWRAP  = 0;
	static u32 MEM_COMMIT_ALIGN_UNWRAP = 0;
	
	static void mem_init(void) {
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		MEM_ALLOC_ALIGN_UNWRAP  = (u32)si.dwAllocationGranularity;
		MEM_COMMIT_ALIGN_UNWRAP = (u32)si.dwPageSize;
	};
	
	u32 mem_alloc_align(void) {
		if (MEM_ALLOC_ALIGN_UNWRAP == 0) mem_init();
		return MEM_ALLOC_ALIGN_UNWRAP;
	};
	
	u32 mem_commit_align(void) {
		if (MEM_COMMIT_ALIGN_UNWRAP == 0) mem_init();
		return MEM_COMMIT_ALIGN_UNWRAP;
	};
#else
	#error "not supported platfrom"
#endif

bool mem_reserve(Memory * mem, usz len, Quota * quota) {
	ASSERT_DEBUG(mem != NULL, "expected [pages]");
	mem->stack = (Stack){0};
	mem->quota = quota;

	if (len == 0) return TRUE;

	uptr const align = mem_alloc_align();
	if (ADD_OVER_DBG(len, align - 1, &len)) return FALSE;
	len &= ~(align - 1);

	#ifdef __linux__
		#ifdef MEM_PROT_DISABLE
			uptr ptr = (uptr)mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		#else
			uptr ptr = (uptr)mmap(NULL, len, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		#endif
		
		if (ptr == -1ULL) return FALSE;
	#elif _WIN32
		uptr ptr = (uptr)VirtualAlloc(NULL, *len, MEM_RESERVE, PAGE_NOACCESS);
		if (ptr == NULL) return FALSE;
	#else
		#error "not supported platfrom"
	#endif

	// ptr + len cannot overflow
	mem->stack = stack_init(ptr, ptr + len);
	return TRUE;
};

bool mem_commit(void * at, usz len) {
#if defined(__linux__)
	#if !defined(MEM_PROT_DISABLE)
		if (at == NULL || len == 0) return FALSE;
		uptr const align = mem_commit_align();
		ASSERT_DEBUG((len & (align - 1)) == 0, "expected that [len] aligned by [commit_align]");
		if (mprotect(at, len, PROT_READ | PROT_WRITE) != 0) return FALSE; // use RW in release
	#endif
#elif _WIN32
	#if !defined(MEM_PROT_DISABLE)
		if (at == NULL || len == 0) return FALSE;
		ptr const align = mem_commit_align();
		ASSERT_DEBUG((len & (align - 1)) == 0, "expected that [len] aligned by [commit_align]");
	#endif
		if (VirtualAlloc(at, len, MEM_COMMIT, PAGE_READWRITE) == NULL) return FALSE;
	#else
		#error "not supported platfrom"
	#endif

	return TRUE;
};

bool mem_release(void * ptr, usz len) {
	#ifndef MEM_PROT_DISABLE
		if (ptr == NULL || len == 0) return TRUE;
	#endif

	#ifdef __linux__
		return munmap(ptr, len) == 0;
	#elif _WIN32
		return VirtualFree(ptr, 0, MEM_RELEASE);
	#else
		#error "not supported platfrom"
	#endif
};

bool mem_set_rw(void * at, usz len) {
	#ifndef MEM_PROT_DISABLE
		if (at == NULL || len == 0) return FALSE;
		uptr const align = mem_commit_align();
		ASSERT_DEBUG((len & (align - 1)) == 0, "expected that [len] aligned by [commit_align]");
	#endif

	#if defined(__linux__)
		if (mprotect(at, len, PROT_READ | PROT_WRITE) != 0) return FALSE; // use RW in release
	#elif _WIN32
		if (VirtualAlloc(at, len, MEM_COMMIT, PAGE_READWRITE) == NULL) return FALSE;
	#else
		#error "not supported platfrom"
	#endif

	return TRUE;
};

bool mem_set_exe(void * at, usz len) {
	#ifndef MEM_PROT_DISABLE
		if (at == NULL || len == 0) return FALSE;
		uptr const align = mem_commit_align();
		ASSERT_DEBUG((len & (align - 1)) == 0, "expected that [len] aligned by [commit_align]");
	#endif

	#if defined(__linux__)
		if (mprotect(at, len, PROT_EXEC | PROT_READ) != 0) return FALSE; // use RW in release
	#elif _WIN32
		if (VirtualAlloc(at, len, MEM_COMMIT, PAGE_EXECUTE_READ) == NULL) return FALSE;
	#else
		#error "not supported platfrom"
	#endif

	return TRUE;
};

bool mem_decommit(void * base, usz len) {
	#ifndef MEM_PROT_DISABLE
		if (base == NULL || len == 0) return TRUE;
	#endif

	#ifdef __linux__
		return mprotect(base, len, PROT_NONE) == 0 && madvise(base, len, MADV_DONTNEED) == 0;
	#elif _WIN32
		return VirtualFree(base, len, MEM_DECOMMIT);
	#else
		#error "not supported platfrom"
	#endif
};

bool mem_split(Memory * src, Memory * dst, usz min_size, usz min_align) {
	ASSERT_DEBUG(src != NULL, "expected [src]");
	ASSERT_DEBUG(dst != NULL, "expected [dst]");
	dst->quota = src->quota;

	usz const page = mem_commit_align();
	usz const align = mem_align_ceil(min_align, page);
	usz const size  = mem_align_ceil(min_size, align);

	return stack_split(&src->stack, &dst->stack, size, align);
};

