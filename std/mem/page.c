#include "std/mem/page.h"
#include "std/core.h"
#include "std/mem/core.h"
#include "std/mem/core.h"
#include "std/mem/cur.h"
#include "std/mem/reg.h"

#if LINUX
	#include "linux/map.h"
#elif WINDOWS
	#include "windows/virt.h"
#endif

// |================================================================================================|
// |> ALIGN                                                                                         |

u32 pages_align(void) {
	#if LINUX
		// TODO: read aux
		return pages_align_atom();
	#elif WINDOWS
		#error "TODO: windows"
	#else
		#error "not supported"
	#endif
};

u32 pages_align_atom(void) {
	#if LINUX
		// TODO: read aux
		return KB(4);
	#elif WINDOWS
		#error "TODO: windows"
	#else
		#error "not supported"
	#endif
};

// |================================================================================================|
// |> PAGES RESERVE/RELEASE                                                                         |

Pages pages_reserve(u64 min_len) {
	u64 const len = mem_align_up(min_len, pages_align());
	if (len == U64_MAX) return PAGES_NIL;

	#if LINUX
		iptr const res = linux_mmap(0, len, LINUX_MAP_PROT_NONE, LINUX_MAP_PRIVATE | LINUX_MAP_ANONYMOUS, -1, 0);
		if (res < 0) return PAGES_NIL;
		uptr const ptr = (uptr)res;
	#elif WINDOWS
		void * const res = VirtualAlloc(0, (size_t)len, WINDOWS_MEM_RESERVE, WINDOWS_PAGE_NOACCESS);
		if (res == 0) return PAGES_NIL;
		uptr const ptr = (uptr)res;
	#else
		#error "not supported"
	#endif

	return PAGES_RAW(ptr, ptr, ptr + len);
};

void pages_release(Pages * pages) {
	#if LINUX
		iptr const res = linux_munmap(pages->beg, pages->end - pages->beg);
		*pages = (Pages){0};
		if (res < 0) return;
	#elif WINDOWS
		VirtualFree((void*)pages.beg, 0, WINDOWS_MEM_RELEASE);
		#error "TODO: windows"
	#else
		#error "not supported"
	#endif
};

// |================================================================================================|
// |> PAGES ALLOC                                                                                   |

Pages pages_chop(Pages * ps, u64 min_size, u32 min_align) {
	if (ps == 0 || !ps->is_valid) return PAGES_NIL;

	u64  const page  = pages_align_atom();
	uptr const size  = mem_align_up(min_size, page);
	uptr const align = mem_align_up(min_align, page);
	if (size == U64_MAX || align == U64_MAX) return PAGES_NIL;

	uptr const beg = mem_align_up(ps->pos, align); uptr end;
	if (ADD_OVER(beg, size, &end) || ps->end <= end) return PAGES_NIL;

	ps->pos = end;
	return PAGES_RAW(beg, beg, end);
};

uptr pages_raw(Pages * ps, u64 size, u32 align) {
	return cur_raw(&ps->cur, size, align);
};

// |================================================================================================|
// |> PAGES PROT                                                                                    |

typedef enum {
	PAGES_PROT_NONE,
	PAGES_PROT_READ,
	PAGES_PROT_READ_WRITE,
	PAGES_PROT_READ_EXEC
} PagesProt;

static b8 set_prot(uptr beg, uptr end, PagesProt prot) {
	uptr const page = pages_align_atom();
	uptr const page_beg = beg & ~(page - 1);
	uptr const page_end = mem_align_up(end, page);
	u64  const page_len = page_end - page_beg;

	#if LINUX
		i32 linux_prot = 0;
		switch (prot) {
			case PAGES_PROT_NONE:       linux_prot = LINUX_MAP_PROT_NONE; break;
			case PAGES_PROT_READ:       linux_prot = LINUX_MAP_PROT_READ; break;
			case PAGES_PROT_READ_WRITE: linux_prot = LINUX_MAP_PROT_READ | LINUX_MAP_PROT_WRITE; break;
			case PAGES_PROT_READ_EXEC:  linux_prot = LINUX_MAP_PROT_READ | LINUX_MAP_PROT_EXEC; break;
		};
		return linux_mprotect(page_beg, page_len, linux_prot) >= 0;

	#elif WINDOWS
		u32 win_prot = 0;
		switch (prot) {
			case PAGES_PROT_NONE:       win_prot = WINDOWS_PAGE_NOACCESS; break;
			case PAGES_PROT_READ:       win_prot = WINDOWS_PAGE_READONLY; break;
			case PAGES_PROT_READ_WRITE: win_prot = WINDOWS_PAGE_READWRITE; break;
			case PAGES_PROT_READ_EXEC:  win_prot = WINDOWS_PAGE_EXECUTE_READ; break;
		};
		
		u32 old_prot;
		return VirtualProtect((void*)page_beg, (size_t)page_len, win_prot, &old_prot) != 0;

	#else
		#error "not supported"
	#endif
};

b8 pages_none(Reg reg)       { return set_prot(reg.ptr, reg.ptr + reg.len, PAGES_PROT_NONE); };
b8 pages_read(Reg reg)       { return set_prot(reg.ptr, reg.ptr + reg.len, PAGES_PROT_READ); };
b8 pages_read_write(Reg reg) { return set_prot(reg.ptr, reg.ptr + reg.len, PAGES_PROT_READ_WRITE); };
b8 pages_read_exec(Reg reg)  { return set_prot(reg.ptr, reg.ptr + reg.len, PAGES_PROT_READ_EXEC); };

// |================================================================================================|
// |> PAGES API                                                                                     |

Reg pages_alloc_api(void * ctx, Reg reg, RegReq req) {
	Pages * ps = ctx;
	if (ps == 0 || !ps->is_valid) return REG_NIL;

	if (!reg.is_valid) {
		if (req.len == 0) return REG_NIL;

		uptr const ptr = pages_raw(ps, req.len, req.align);
		if (ptr == 0) return REG_NIL;

		uptr const page = pages_align_atom();
		uptr const page_beg = ptr & ~(page - 1);
		uptr const page_end = mem_align_up(ptr + req.len, page);
		u64  const page_len = page_end - page_beg;

		if (!pages_read_write(REG(page_beg, page_len))) return REG_NIL;
		return REG(ptr, req.len);
	};

	uptr const reg_end = reg.ptr + reg.len;

	if (req.dir == REG_DIR_UP) {
		if (reg_end == ps->cur.pos) {
			if (req.len == 0) {
				ps->cur.pos = reg.ptr;
				return REG_NIL;
			};

			if (req.len <= reg.len) {
				ps->cur.pos = reg.ptr + req.len;
				return REG(reg.ptr, req.len);
			};

			u64 const extra_len = req.len - reg.len;
			uptr const new_pos = reg_end + extra_len;
			if (new_pos <= ps->cur.end) {
				uptr const page = pages_align_atom();
				uptr const current_page_end = mem_align_up(reg_end, page);
				uptr const new_page_end     = mem_align_up(new_pos, page);

				if (new_page_end > current_page_end) {
					Reg extra_zone = REG(current_page_end, new_page_end - current_page_end);
					if (!pages_read_write(extra_zone)) return REG_NIL;
				};

				ps->cur.pos = new_pos;
				return REG(reg.ptr, req.len);
			};
		};
	} else {
		if (reg.ptr == ps->cur.pos) {
			if (req.len == 0) {
				ps->cur.pos = reg_end;
				return REG_NIL;
			};

			if (req.len <= reg.len) {
				u64 const shrink_len = reg.len - req.len;
				ps->cur.pos = reg.ptr + shrink_len;
				return REG(ps->cur.pos, req.len);
			};

			u64 const extra_len = req.len - reg.len;
			if (ps->cur.pos >= ps->beg + extra_len) {
				uptr const new_pos = ps->cur.pos - extra_len;
				uptr const page = pages_align_atom();
				uptr const current_page_beg = reg.ptr & ~(page - 1);
				uptr const new_page_beg     = new_pos & ~(page - 1);

				if (new_page_beg < current_page_beg) {
					Reg extra_zone = REG(new_page_beg, current_page_beg - new_page_beg);
					if (!pages_read_write(extra_zone)) return REG_NIL;
				};

				ps->cur.pos = new_pos;
				return REG(new_pos, req.len);
			};
		};
	};

	if (req.len == 0) return REG_NIL;

	uptr const new_ptr = pages_raw(ps, req.len, req.align);
	if (new_ptr == 0) return REG_NIL;

	uptr const page = pages_align_atom();
	uptr const page_beg = new_ptr & ~(page - 1);
	uptr const page_end = mem_align_up(new_ptr + req.len, page);
	u64  const page_len = page_end - page_beg;

	if (!pages_read_write(REG(page_beg, page_len))) return REG_NIL;

	u64 const copy_len = reg.len < req.len ? reg.len : req.len;
	if (req.dir == REG_DIR_UP) {
		memcpy((void*)new_ptr, (void*)reg.ptr, copy_len);
	} else {
		u64 const offset = req.len - copy_len;
		memcpy((void*)(new_ptr + offset), (void*)reg.ptr, copy_len);
	};

	return REG(new_ptr, req.len);
};

Reg pages_no_state_api(void * ctx, Reg reg, RegReq req) {
	UNUSED(ctx);
	uptr const page_size = pages_align_atom();

	if (req.len == 0) {
		if (reg.is_valid) {
			u64 const release_len = mem_align_up(reg.len, page_size);
			#if LINUX
				linux_munmap(reg.ptr, release_len);
			#elif WINDOWS
				VirtualFree((void*)reg.ptr, 0, MEM_RELEASE);
			#endif
		};
		return REG_NIL;
	};

	if (!reg.is_valid) {
		u64 const alloc_len = mem_align_up(req.len, page_size);
		if (alloc_len == U64_MAX) return REG_NIL;

		#if LINUX
			iptr const res = linux_mmap(
				0, alloc_len,
				LINUX_MAP_PROT_READ | LINUX_MAP_PROT_WRITE,
				LINUX_MAP_PRIVATE | LINUX_MAP_ANONYMOUS,
				-1, 0
			);
			if (res < 0) return REG_NIL;
			return REG((uptr)res, req.len);
		#elif WINDOWS
			void* const res = VirtualAlloc(0, (size_t)alloc_len, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
			if (res == 0) return REG_NIL;
			return REG((uptr)res, req.len);
		#endif
	};

	RegReq new_alloc_req = REG_REQ(req.len, req.align, req.dir);
	Reg new_reg = pages_no_state_api(0, REG_NIL, new_alloc_req);
	if (!new_reg.is_valid) return REG_NIL;

	u64 const copy_len = reg.len < req.len ? reg.len : req.len;
	if (req.dir == REG_DIR_UP) {
		memcpy((void*)new_reg.ptr, (void*)reg.ptr, copy_len);
	} else {
		u64 const offset = req.len - copy_len;
		memcpy((void*)(new_reg.ptr + offset), (void*)reg.ptr, copy_len);
	};

	pages_no_state_api(0, reg, REG_REQ_FREE);
	return new_reg;
};

RegMan pages_pinned(Pages * ps) { return REG_MAN(ps, pages_alloc_api); };
RegMan pages_no_state(void) { return REG_MAN(0, pages_no_state_api); };

