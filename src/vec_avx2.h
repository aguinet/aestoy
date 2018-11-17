#ifndef AESTOY_VEC_AVX2_H
#define AESTOY_VEC_AVX2_H

#include <immintrin.h>

namespace {

using Vec16 = __m128i;

Vec16 vec_load(uint8_t const* V) {
  return _mm_load_si128((const __m128i*)V);
}

Vec16 vec_load(std::array<uint8_t, 16> const& V) {
  return vec_load(&V[0]);
}

Vec16 vec_loadu(uint8_t const* V) {
  return _mm_loadu_si128((const __m128i*)V);
}

Vec16 vec_loadu(std::array<uint8_t, 16> const& V) {
  return vec_loadu(&V[0]);
}

void vec_store(void* Out, Vec16 V) {
  _mm_store_si128((__m128i*) Out, V);
}

__attribute__((noinline)) void vec_store_(void* Out, Vec16 V) {
  _mm_store_si128((__m128i*) Out, V);
}

void vec_storeu(void* Out, Vec16 V) {
  _mm_storeu_si128((__m128i*) Out, V);
}

Vec16 xor_(Vec16 A, Vec16 B) {
  return _mm_xor_si128(A,B);
}

Vec16 vec_set(uint32_t A, uint32_t B, uint32_t C, uint32_t D) {
  return _mm_set_epi32(A, B, C, D);
}

Vec16 vec_set(std::array<uint8_t, 16> V) {
  return _mm_set_epi8(V[0],V[1],V[2],V[3],V[4],V[5],V[6],V[7],V[8],V[9],V[10],V[11],V[12],V[13],V[14],V[15]);
}

template <size_t... Idxes>
Vec16 vec_shuffle_u8(Vec16 V) {
  return _mm_shuffle_epi8(V, _mm_set_epi8(Idxes...));
}

template <size_t I0, size_t I1, size_t I2, size_t I3>
Vec16 vec_gather_u32(uint32_t const* BaseAddr, Vec16 V) {
  V = vec_shuffle_u8<
    0xFF,0xFF,0xFF,I0,
    0xFF,0xFF,0xFF,I1,
    0xFF,0xFF,0xFF,I2,
    0xFF,0xFF,0xFF,I3>(V);
  return _mm_i32gather_epi32((const int*)BaseAddr, V, sizeof(uint32_t));
}

#if 0
void vec_transpose(Vec16& V0, Vec16& V1, Vec16& V2, Vec16& V3)
{
  _MM_TRANSPOSE4_PS(
    reinterpret_cast<__m128>(V0),
    reinterpret_cast<__m128>(V1),
    reinterpret_cast<__m128>(V2),
    reinterpret_cast<__m128>(V3));
}
#endif

} // anonymous

#endif
