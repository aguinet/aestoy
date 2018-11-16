#ifndef AESTOY_VEC_H
#define AESTOY_VEC_H

#ifdef __AVX2__
#define AESTOY_ENABLE_AVX2_IMPL
#include "vec_avx2.h"
#endif

template <class... CIdxes>
Vec16 vec_shuffle_u8(Vec16 V, CIdxes... Idxes) {
  return __builtin_shufflevector(V, V, Idxes...);
}

#endif
