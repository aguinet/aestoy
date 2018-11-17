#ifndef AESTOY_VEC_H
#define AESTOY_VEC_H

#ifdef __AVX2__
#define AESTOY_ENABLE_VECTOR_IMPL
#include "vec_avx2.h"
#elif defined(__ARM_NEON__)
#define AESTOY_ENABLE_VECTOR_IMPL
#include "vec_neon.h"
#endif

#endif
