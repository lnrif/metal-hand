#include "std/rnd/core.h"

u64 rnd_splitmix64(u64 * seed) {
	u64 z = (*seed += 0x9e3779b97f4a7c15ULL);
	z = (z ^ (z >> 30u)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27u)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31u);
};

u32 rnd_pcg(u32 * seed) {
	u32 v = *seed;
	*seed = v * 747796405u + 2891336453u;
	u32 w = ((v >> ((v >> 28u) + 4u)) ^ v) * 277803737u;
	return (w >> 22u) ^ w;
};

f32 rnd_uni(u32 * seed) {
	union { u32 u; f32 f; } pun;
	pun.u = (rnd_pcg(seed) >> 9) | 0x3f800000;
	return pun.f - 1.0f;
};

f32 rnd_bi(u32 * seed) {
	union { u32 u; f32 f; } pun;
	pun.u = (rnd_pcg(seed) >> 9) | 0x3f800000;
	return pun.f * 2.0f - 3.0f;
};

