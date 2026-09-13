#include "mem/quota.h"

#define TERM_RED    "\x1b[0;1;31m"
#define TERM_ORANGE "\x1b[0;1;38;5;214m"
#define TERM_BLUE   "\x1b[0;1;34m"
#define TERM_GREEN  "\x1b[0;1;32m"
#define TERM_WHITE  "\x1b[0;1;37m"
// #define TERM_RED "\x1b[0;1;4;37m" // <- attention
#define TERM_GREY   "\x1b[0;38;5;8m"
#define TERM_CYAN   "\x1b[0;36m"
#define TERM_PURPLE "\x1b[0;1;35m"
#define TERM_RESET  "\x1b[0m"

#if 1
	const QuotaTheme QUOTA_THEME_DEFAULT = (QuotaTheme){
		.name = STR(TERM_BLUE),
		.bar = {
			.used     = STR(TERM_ORANGE),
			.commited = STR(TERM_RED),
			.virtual  = STR(TERM_GREY),
		},
	};
#else
	const QuotaTheme QUOTA_THEME_DEFAULT = (QuotaTheme){
		.name = STR(""),
		.bar = {
			.used     = STR(""),
			.commited = STR(""),
			.virtual  = STR(""),
		},
	};
#endif

bool quota_acquare_parents(Quota * const quota, usz n) {
	Quota * node = quota;
	bool failed = FALSE;
	
	while (node != NULL) {
		// printf("Parent { now: %llu, peak: %llu, limit: %llu }\n", node->now, node->peak, node->limit);
		// 1) can acquare?
		usz now = node->now;
		if (ADD_OVER(now, n, &now) || now > node->limit) {
			failed = TRUE;
			break;
		};
		// 3) set to parent
		node = node->parent;
	};
	
	if (failed) return FALSE;
	
	node = quota;
	while (node != NULL) {
		// 1) can acquare
		node->now += n;
		node->peak = MAX(node->peak, node->now);
		// 2) set to parent
		node = node->parent;
	};
	
	return TRUE;
};

bool quota_acquare(Quota * quota, usz n) {
	// 1) no qouta - no limits!
	if (quota == NULL) return TRUE;

	// 2) can not acquare parents - fail
	if (!quota_acquare_parents(quota, n)) return FALSE;
	// // 3) check local quota
	// {
	// 	// 2.1) can acquare?
	// 	usz now = quota->now;
	// 	if (ADD_OVER(now, n, &now)) return FALSE;
	// 	// 2.2) acquare accepted
	// 	quota->now = now;
	// 	quota->peak = MAX(quota->peak, quota->now);
	// };
	//
	return TRUE;
};

void quota_release_parents(Quota * const quota, usz n) {
	Quota * node = quota;
	while (node != NULL) {
		bool overflow = SUB_OVER(node->now, n, &node->now);
		ASSERT_DEBUG(!overflow, "quota underflow, trying to free more than allocated!");
		node = node->parent;
	};
};

void quota_release(Quota * quota, usz n) {
	// 1) no qouta - no limits!
	if (quota == NULL) return;
	// 2) release parents
	quota_release_parents(quota->parent, n);
	// 3) release local
	{
		bool const overflow = SUB_OVER(quota->now, n, &quota->now);
		ASSERT_DEBUG(!overflow, "quota underflow, trying to free more than allocated!");
	};
};

void quota_print(
	Quota const * quota,
	QuotaTheme const * theme,
	StrOpt name
) {
	if (quota == NULL) return;
	
	if (str_sane(name)) printf(
		"%.*s" "%.*s ",
		(u32)theme->name.len, theme->name.ptr,
		(u32)name.len, name.ptr
	);

	printf("%.*s" "[", (u32)theme->bar.virtual.len, theme->bar.virtual.ptr);
	usz const step = quota->limit >> 3;
	usz i = step;

	printf("%.*s", (u32)theme->bar.used.len, theme->bar.used.ptr);
	for (;i < quota->now; i += step) printf("x");
	
	printf("%.*s", (u32)theme->bar.commited.len, theme->bar.commited.ptr);
	for (;i <= quota->peak; i += step) printf("_");
	
	if (quota->peak != 0 && i < quota->limit) {
		i += step;
		printf("|");
	};

	printf("%.*s", (u32)theme->bar.virtual.len, theme->bar.virtual.ptr);
	for (;i <= quota->limit; i += step) printf("_");
	
	u32 prec = 0;
	Str scale_str = STR("KB");
	f64 scale_div = 1024.0;
	if (quota->limit >= MB(1)) {
		prec       = 1;
		scale_str  = STR("MB");
		scale_div *= 1024.0;
	};

	printf("%.*s] ", (u32)theme->bar.virtual.len, theme->bar.virtual.ptr);

	// {
	// 	u32 n = 0;
	// 	usz x = quota->peak * 100 / quota->limit;
	// 	while (x > 0) { n += 1; x /= 10; };
	// 	printf(
	// 		"%.*s" "%*llu" " "
	// 		"%.*s" "%*llu" " "
	// 		"%.*s" "[%%]",
	// 		(u32)theme->bar.used    .len, theme->bar.used    .ptr, n, quota->now  * 100 / quota->limit,
	// 		(u32)theme->bar.commited.len, theme->bar.commited.ptr, n, quota->peak * 100 / quota->limit,
	// 		(u32)theme->bar.virtual .len, theme->bar.virtual .ptr
	// 	);
	// };
	
	// printf(" ");

	{
		u32 n = (prec == 0) ? 0 : prec + 1;
		usz x = (usz)((f64)quota->peak / scale_div);
		while (x > 0) { n += 1; x /= 10; };
		printf(
			"%.*s" "%*.*f" " "
			"%.*s" "%*.*f" " "
			"%.*s" "%*.*f" " "
			"%.*s" "[%.*s]",
			(u32)theme->bar.used    .len, theme->bar.used    .ptr, n, prec, (f64)quota->now   / scale_div,
			(u32)theme->bar.commited.len, theme->bar.commited.ptr, n, prec, (f64)quota->peak  / scale_div,
			(u32)theme->name        .len, theme->name        .ptr, n, prec, (f64)quota->limit / scale_div,
			(u32)theme->bar.virtual .len, theme->bar.virtual .ptr,
			(u32)scale_str          .len, scale_str          .ptr
		);
	};

	
	printf("\n" TERM_RESET);
};
