#ifndef RK_MEM_QUOTES_H
#define RK_MEM_QUOTES_H

typedef struct MemQuota MemQuota;

#include "mem/pool.h"

struct MemQuota {
	MemPool pool;
};

#endif // !RK_MEM_QUOTES_H
