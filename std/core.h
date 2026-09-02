#ifndef STD_CORE_H
#define STD_CORE_H

#include <stdalign.h>

#define STATIC_ASSERT(cond, args...) _Static_assert(cond, "" args)

// |================================================================================================|
// |> PLATFORM                                                                                      |
// |================================================================================================|

#if defined(__x86_64__)
	#define X86_64 1
#elif defined(__aarch64__)
	#define ARM64 1
#else
	#error "unsupported architecture"
#endif

#if defined(__linux__)
	#define LINUX 1
#elif defined(_WIN32)
	#define WINDOWS 1
#else
	#error "unsupported OS"
#endif

// |================================================================================================|
// |> TYPES                                                                                         |
// |================================================================================================|

#define TYPE_EQ(a, b) __builtin_types_compatible_p(typeof(a), typeof(b))
#define TYPE_IS_SIGNED(x) ((typeof(x))-1 < 0)
#define IS_TYPE(expr, type) _Generic((expr), type: 1, default: 0)
#define UNUSED(x...) (void)(x)
#define AS(T, expr) ((T*)(void*)(expr))

// |================================================================================================|
// |> UNSIGNED                                                                                      |

typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;

typedef u8 u8z; // null terminated
STATIC_ASSERT(sizeof(u8z) == 1);

STATIC_ASSERT(sizeof(u8)  == 1);
STATIC_ASSERT(sizeof(u16) == 2);
STATIC_ASSERT(sizeof(u32) == 4);
STATIC_ASSERT(sizeof(u64) == 8);

#define U2_MAX  ((u8)                0x4ULL)
#define U4_MAX  ((u8)                0xFULL)
#define U8_MAX  ((u8)               0xFFULL)
#define U16_MAX ((u16)            0xFFFFULL)
#define U24_MAX ((u32)          0xFFFFFFULL)
#define U32_MAX ((u32)        0xFFFFFFFFULL)
#define U40_MAX ((u64)      0xFFFFFFFFFFULL)
#define U48_MAX ((u64)    0xFFFFFFFFFFFFULL)
#define U56_MAX ((u64)  0xFFFFFFFFFFFFFFULL)
#define U64_MAX ((u64)0xFFFFFFFFFFFFFFFFULL)

// |================================================================================================|
// |> SIGNED                                                                                        |

typedef signed char      i8;
typedef signed short     i16;
typedef signed int       i32;
typedef signed long long i64;

STATIC_ASSERT(sizeof(i8)  == 1);
STATIC_ASSERT(sizeof(i16) == 2);
STATIC_ASSERT(sizeof(i32) == 4);
STATIC_ASSERT(sizeof(i64) == 8);

#define I8_MAX  ((i8) 0x7F)
#define I16_MAX ((i16)0x7FFF)
#define I32_MAX ((i32)0x7FFFFFFF)
#define I64_MAX ((i64)0x7FFFFFFFFFFFFFFFLL)

#define I8_MIN  ((i8) (-I8_MAX - 1))
#define I16_MIN ((i16)(-I16_MAX - 1))
#define I32_MIN ((i32)(-I32_MAX - 1))
#define I64_MIN ((i64)(-I64_MAX - 1LL))

// |================================================================================================|
// |> FLOAT                                                                                         |

typedef float f32;
typedef double f64;

STATIC_ASSERT(sizeof(f32) == 4);
STATIC_ASSERT(sizeof(f64) == 8);

// |================================================================================================|
// |> BOOL                                                                                          |

typedef enum: u8 { false, true } b8;
#define bool(x) ((b8)((x) != 0))

typedef u32 b32;

STATIC_ASSERT(sizeof(b8)  == 1);
STATIC_ASSERT(sizeof(b32) == 4);

// |================================================================================================|
// |> PTR                                                                                           |

typedef u64 uptr;
typedef i64 iptr;
STATIC_ASSERT(sizeof(uptr) == sizeof(void*));
STATIC_ASSERT(sizeof(iptr) == sizeof(void*));

// |================================================================================================|
// |> MEM                                                                                           |
// |================================================================================================|

#define KB(x) ((u64)(x) << 10)
#define MB(x) ((u64)(x) << 20)
#define GB(x) ((u64)(x) << 30)
#define TB(x) ((u64)(x) << 40)

#define LEN(arr...) ((u64)(sizeof(arr) / sizeof((arr)[0])))

// |================================================================================================|
// |> BUILTIN                                                                                       |
// |================================================================================================|

#define ALLOCA(len...) ((void*)__builtin_alloca(len))
#define CTZ(n...) (u64)(__builtin_ctzll(n))
#define ADD_OVER(a, b, r) __builtin_add_overflow(a, b, r)
#define SUB_OVER(a, b, r) __builtin_sub_overflow(a, b, r)
#define MUL_OVER(a, b, r) __builtin_mul_overflow(a, b, r)

#if defined(DEBUG)
	#define UNREACHABLE __builtin_unreachable()
#else
	#define UNREACHABLE __builtin_unreachable()
#endif

// |================================================================================================|
// |> ATTRIBUTES                                                                                    |
// |================================================================================================|

#define MUST_USE   __attribute__((warn_unused_result))
#define USED       __attribute__((used))
#define NAKED      __attribute__((naked))
#define ALIGNED(N) __attribute__((aligned(N)))

#define NO_RETURN   __attribute__((noreturn))
#define NO_BUILTIN(args...) __attribute__((no_builtin(args)))

#define IMPORT __attribute__((dllimport))
#define EXPORT __attribute__((dllexport))

// |================================================================================================|
// |> MACRO MAGIC                                                                                   |
// |================================================================================================|

#define _CAT(a, b) a ## b
#define CAT(a, b) _CAT(a, b)

#define _STRIFY(x) #x
#define STRIFY(x) _STRIFY(x)

#define UNIQ(name) CAT(name, __COUNTER__)

#define VARIADIC_2(    _2, _1, f, ...) f
#define VARIADIC_3(_3, _2, _1, f, ...) f

// |================================================================================================|
// |> MATH                                                                                          |
// |================================================================================================|

#define DEBUG

#ifdef DEBUG
    #define ADD_OVER_DBG(a, b, r) ADD_OVER(a, b, r)
    #define SUB_OVER_DBG(a, b, r) SUB_OVER(a, b, r)
    #define MUL_OVER_DBG(a, b, r) MUL_OVER(a, b, r)
#else
    #define ADD_OVER_DBG(a, b, r) (*(r) = (a) + (b), false)
    #define SUB_OVER_DBG(a, b, r) (*(r) = (a) - (b), false)
    #define MUL_OVER_DBG(a, b, r) (*(r) = (a) * (b), false)
#endif

#define _MIN(_a, _b, ID) ({ \
	typeof(_a) CAT(a, ID) = (_a); \
	typeof(_b) CAT(b, ID) = (_b); \
	CAT(a, ID) < CAT(b, ID) ? CAT(a, ID) : CAT(b, ID); \
})

#define _MAX(_a, _b, ID) ({ \
	typeof(_a) CAT(a, ID) = (_a); \
	typeof(_b) CAT(b, ID) = (_b); \
	CAT(a, ID) > CAT(b, ID) ? CAT(a, ID) : CAT(b, ID); \
})

#define _DIV_CEIL(_x, _d, ID) ({ \
	STATIC_ASSERT(TYPE_EQ(_x, _d), "types mismatch"); \
	STATIC_ASSERT(!TYPE_IS_SIGNED(_x), "DIV_CEIL expected unsigned type"); \
	typeof(_x) CAT(x, ID) = (_x); \
	typeof(_d) CAT(d, ID) = (_d); \
	(CAT(x, ID) / CAT(d, ID)) + (CAT(x, ID) % CAT(d, ID) != 0); \
})

#define MIN(a, b) _MIN(a, b, UNIQ(_urm_min_))
#define MAX(a, b) _MAX(a, b, UNIQ(_urm_max_))
#define CLAMP(x, min, max) MAX(min, MIN(x, max))

#define DIV_CEIL(x, d) _DIV_CEIL(x, d, UNIQ(_urm_div_ceil_))

#endif // !STD_CORE_H
