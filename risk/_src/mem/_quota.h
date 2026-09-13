#ifndef QUOTA_H
#define QUOTA_H

#include "core.h"

typedef u16 quota_idx;

/// Measured in pages: [now] <= [peak] <= [limit].
#define QUOTA_HEADER_FIELDS \
	u32 now; \
	u32 peak; \
	u32 limit; \
	u16 parent_offset; \
	u16 subs_count

typedef struct { QUOTA_HEADER_FIELDS; } quota_header;

#define QUOTA_HEADER \
	union { \
		struct { QUOTA_HEADER_FIELDS; }; \
		quota_header header; \
	}

typedef struct {
	QUOTA_HEADER;
	u16 sub_offsets[1];
} quota1;

typedef struct {
	QUOTA_HEADER;
	u16 sub_offsets[2];
} quota2;


#endif // !QUOTA_H
