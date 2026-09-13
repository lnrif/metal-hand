#ifndef RK_MEM_QUOTA_H
#define RK_MEM_QUOTA_H

#include "str/str.h"

typedef struct Quota Quota;

// now <= peak <= limit
struct Quota {
	usz now;
	usz peak;
	usz limit;
	Quota * parent; // optional
};

typedef struct {
	Str name;
	struct {
		Str used;
		Str commited;
		Str virtual;
	} bar;
} QuotaTheme;

extern const QuotaTheme QUOTA_THEME_DEFAULT;

#define quota_init(_limit, _parent) ((Quota){.now = 0, .peak = 0, .limit = (_limit), .parent = (_parent)})
#define quota_node(parent) quota_init((parent)->limit, parent)

bool quota_acquare(Quota * quota, usz n);
void quota_release(Quota * quota, usz n);

void quota_print(Quota const * quota, QuotaTheme const * theme, StrOpt name);


#endif // RK_MEM_QUOTA_H
