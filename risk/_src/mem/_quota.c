#ifndef URM_QUOTA_C
#define URM_QUOTA_C

#include "../core.c"
#include "../str.c"
#include <stdio.h>

#define URM_KIB(x) ((urm_usz)(x) << 10)
#define URM_MIB(x) ((urm_usz)(x) << 20)
#define URM_GIB(x) ((urm_usz)(x) << 30)

// ================================================================================
// [INTERFACE]
// ================================================================================

typedef struct {
	urm_u16 idx;
	urm_u16 gen;
} urm_mem_quota_id;

#define URM_MEM_QUOTA_ID_INVALID \
	((urm_mem_quota_id){ \
		.idx = URM_U16_MAX, \
		.gen = URM_U16_MAX, \
	})

static inline
urm_bool urm_mem_quota_id_sane(urm_mem_quota_id id);

typedef struct {
	urm_u32 now;
	urm_u32 peak;
	urm_u32 limit;
	urm_mem_quota_id next;
	urm_mem_quota_id child;
	urm_mem_quota_id parent;
} urm_mem_quota;

typedef struct {
	urm_mem_quota * free;
	urm_mem_quota * ptr;
	urm_usz len;
} urm_mem_quotes;

typedef struct {
	urm_str name;
	struct {
		urm_str used;
		urm_str commited;
		urm_str virtual;
	} bar;
} urm_quota_theme;

// ================================================================================
// [IMPLEMENTATION]
// ================================================================================

static inline
urm_bool urm_mem_quota_id_sane(urm_mem_quota_id id) {
	return id.idx != URM_U16_MAX && id.gen != URM_U16_MAX;
};

#endif // URM_QUOTA_C
