#ifndef RK_TERM_H
#define RK_TERM_H

// #include "core/types.h"
#include "mem/mem.h"

#define TERM_RED    "\x1b[0;1;4;37m"
#define TERM_ORANGE "\x1b[0;1;38;5;214m"
#define TERM_BLUE   "\x1b[0;1;34m"
#define TERM_GREEN  "\x1b[0;1;32m"
#define TERM_GREY   "\x1b[0;38;5;8m"
#define TERM_CYAN   "\x1b[0;36m"
#define TERM_PURPLE "\x1b[0;1;35m"
#define TERM_RESET  "\x1b[0m"


#define TERM_ERROR(virt, kind, default_text, fmt, ...) do { \
	fprintf(stderr, ASSERT_RED "%s: " ASSERT_WHITE fmt "\n", kind, ##__VA_ARGS__); \
	fprintf(stderr, ASSERT_CYAN " --> %s:%d " ASSERT_PURPLE "(%s)\n" ASSERT_RESET, __FILE__, __LINE__, __func__); \
	abort(); \
} while (0)

// bool term_error(VirtUp * virt) {
// 	fprintf(stderr, TERM_RED "%s: " ASSERT_WHITE fmt "\n", kind, ##__VA_ARGS__); \
// 	fprintf(stderr, ASSERT_CYAN " --> %s:%d " ASSERT_PURPLE "(%s)\n" ASSERT_RESET, __FILE__, __LINE__, __func__); \
// 	abort(); \
// };

#endif // !RK_TERM_H
