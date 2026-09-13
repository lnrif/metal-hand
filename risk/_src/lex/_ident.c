#include "lex/ident.h"

#include "assert.h"
#include "mem/vec.h"
#include "mem/virt.h"
#include <string.h>

static u64 hash_mix(u64 h) {
	h ^= h >> 33;
	h *= 0xff51afd7ed558ccdULL;
	h ^= h >> 33;
	h *= 0xc4ceb9fe1a85ec53ULL;
	h ^= h >> 33;
	return h;
};

static u64 get_hash(Str v) {
	u8 const * p = v.ptr;
	u64 len = v.len;
	u64 h = 0xDEAD;

	while (len >= 8) {
		u64 k;
		__builtin_memcpy(&k, p, 8);
		
		k *= 0xbf58476d1ce4e5b9ULL;
		k ^= k >> 24;
		k *= 0x94d049bb133111ebULL;
		
		h ^= k;
		h *= 0xff51afd7ed558ccdULL;
		
		p += 8;
		len -= 8;
	};

	if (len > 0) {
		u64 tail = 0;
		for (u32 i = 0; i < len; i++) {
			tail |= ((u64)p[i]) << (i * 8);
		}
		h ^= tail;
		h *= 0xc4ceb9fe1a85ec53ULL;
	};

	return hash_mix(h ^ len);
};

LexIdents lex_ident_init(Memory * mem) {
	ASSERT_DEBUG(mem != NULL, "expected [mem]");
	return (LexIdents){
		.idents.virt_vec = virt_vec_init(LexIdentVec, mem, .size = MB(32)),
		.raw = virt_stack_init(mem),
	};
};

LexIdent lex_ident_add(LexIdents * map, Str string) {
	ASSERT_DEBUG(map != NULL, "expected [map]");

	// use hash is kind a useless with linear search...
	u64 const hash = get_hash(string);

	vec_iter(&map->idents, entry, i, {
		if (entry->hash != hash) continue;

		Str const s = lex_ident_get(map, (LexIdent)i);
		if (!str_eq(s, string)) continue;

		return (LexIdent)i;
	});

	LexIdent const id  = (LexIdent)map->idents.len;
	u32    const len = (u32)string.len;

	u8 * ptr = virt_stack_array(&map->raw, u8, len + 1);
	if (ptr == NULL || !virt_stack_commit(&map->raw)) return LEX_IDENT_ID_INVALID;
	memcpy(ptr, string.ptr, len); ptr[len] = '\0';

	u32 pos = (u32)(map->raw.stack.end - (uptr)ptr);
	LexIdentEntry entry = (LexIdentEntry){.hash = hash, .pos = pos, .len = len};
	if (!VIRT_VEC_PUSH(&map->idents, entry)) return LEX_IDENT_ID_INVALID;
	
	return id;
};

Str lex_ident_get(LexIdents const * map, LexIdent id) {
	ASSERT_DEBUG(map != NULL, "expected [map]");
	ASSERT_DEBUG(id < map->idents.len, "invalid [id]");

	LexIdentEntry const entry = map->idents.ptr[id];
	u8 * const ptr = (u8*)(map->raw.stack.end - entry.pos);
	return (Str){.ptr = ptr, .len = entry.len};
};

