#pragma once

//sse3
//#include <pmmintrin.h>
#include <immintrin.h>   // (Meta-header, for GCC only)
#include <omp.h>

void memcpy_sse(void* dest, const void* source, size_t total_length);