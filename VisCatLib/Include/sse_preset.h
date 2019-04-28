#pragma once

//sse3
//#include <pmmintrin.h>
#include <immintrin.h>   // (Meta-header, for GCC only)
#include <omp.h>

#include "Enums.h"

//32bit float
static __m128 g_zero4 = _mm_set1_ps(0);
static __m128 g_one4 = _mm_set1_ps(1);
static __m128 g_negOne4 = _mm_set1_ps(-1);

static __m128 g_infinite4 = _mm_set1_ps(10000000.0f);
static __m128 g_negInfinite4 = _mm_set1_ps(-10000000.0f);

static __m128 g_epsilon4 = _mm_set1_ps(0.001953125);
static __m128 g_negEpsilon4 = _mm_set1_ps(-0.001953125);

static __m128 g_constNumbers[5] = {
									_mm_set1_ps(0),
									_mm_set1_ps(1),
									_mm_set1_ps(2),
									_mm_set1_ps(3),
									_mm_set1_ps(4)
								  };

static __m128 g_0123 = _mm_set_ps(3,2,1,0);
static __m128 g_3210 = _mm_set_ps(0,1,2,3);

static unsigned int uint_max = 0xffffffff;
static __m128 g_ffffffff4 = _mm_set1_ps(*((float*)(&uint_max)));

static __m128 g_invTileSize = _mm_set1_ps(1.0f/TILE_BUFFER_WIDTH);

//32bit int
static __m128i g_intZero4 = _mm_set1_epi32(0);
static __m128i g_int0123 = _mm_set_epi32(3,2,1,0);
static __m128i g_int3210 = _mm_set_epi32(0,1,2,3);