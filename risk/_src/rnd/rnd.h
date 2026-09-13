#ifndef RK_RND_H
#define RK_RND_H

#include "core/types.h"

/// returns random u64 in range 0..=U64_MAX
u64 rnd_splitmix64(u64 * seed);
/// returns random u32 in range 0..=U32_MAX
u32 rnd_pcg(u32 * seed);
/// returns random f32 in range 0.0..<1.0
f32 rnd_uni(u32 * seed);
/// returns random f32 in range -1.0..<1.0
f32 rnd_bi(u32 * seed);

#endif // !RK_RND_H
