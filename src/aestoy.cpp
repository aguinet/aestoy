#include <array>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <aestoy/aestoy.h>
#include "aes_csts.h"
#include "intmem.h"
#include "vec.h"

using namespace intmem;

namespace aestoy {

namespace {

template <class T, size_t N>
__attribute__((always_inline)) std::array<T, N>& operator^=(std::array<T,N>& __restrict A, std::array<T,N> const& __restrict B)
{
  for (size_t i = 0; i < N; ++i) {
    A[i] ^= B[i];
  }
  return A;
}

using RkBlock = std::array<uint8_t, 4>;
RkBlock* GetRksBlock(RoundKey* RKeys, size_t n)
{
  size_t idx = n*4;
  return (RkBlock*) &RKeys[idx/16][idx%16];
}

template <class Container>
void ApplySB(Container& C)
{
  for (uint8_t& V: C) {
    V = RJD_SBOX[V];
  }
}

template <size_t Idx0_, size_t Idx1_, size_t Idx2_, size_t Idx3_>
struct MixCIdx {
  static constexpr size_t Idx0 = Idx0_;
  static constexpr size_t Idx1 = Idx1_;
  static constexpr size_t Idx2 = Idx2_;
  static constexpr size_t Idx3 = Idx3_;
};

using MixC0 = MixCIdx< 0, 5,10,15>;
using MixC1 = MixCIdx< 4, 9,14, 3>;
using MixC2 = MixCIdx< 8,13, 2, 7>;
using MixC3 = MixCIdx<12, 1, 6,11>;

static constexpr std::array<uint8_t, 16> SR = {
  MixC3::Idx3, MixC3::Idx2, MixC3::Idx1, MixC3::Idx0,
  MixC2::Idx3, MixC2::Idx2, MixC2::Idx1, MixC2::Idx0,
  MixC1::Idx3, MixC1::Idx2, MixC1::Idx1, MixC1::Idx0,
  MixC0::Idx3, MixC0::Idx2, MixC0::Idx1, MixC0::Idx0};

template <class MixC>
uint32_t SRSBMixC(uint8_t const* S)
{
  return RJD_Te0[S[MixC::Idx0]] ^
         rol<8>(RJD_Te0[S[MixC::Idx1]]) ^
         rol<16>(RJD_Te0[S[MixC::Idx2]]) ^
         rol<24>(RJD_Te0[S[MixC::Idx3]]);
}

template <class MixC>
void SRSB(uint8_t* Out, uint8_t const* S)
{
  Out[0] = (RJD_Te0[S[MixC::Idx0]] >> 8) & 0xFF;
  Out[1] = (RJD_Te0[S[MixC::Idx1]] >> 8) & 0xFF;
  Out[2] = (RJD_Te0[S[MixC::Idx2]] >> 8) & 0xFF;
  Out[3] = (RJD_Te0[S[MixC::Idx3]] >> 8) & 0xFF;
}

using AESState = std::array<uint8_t, 16>;


} // anonymous

#ifdef AESTOY_ENABLE_VECTOR_IMPL
namespace {
Vec16 SRSBMixC_all(Vec16 S)
{
  auto S0 = vec_gather_u32<
    MixC3::Idx0,
    MixC2::Idx0,
    MixC1::Idx0,
    MixC0::Idx0>(&RJD_Te0[0], S);

  auto S1 = vec_gather_u32<
    MixC3::Idx1,
    MixC2::Idx1,
    MixC1::Idx1,
    MixC0::Idx1>(&RJD_Te1[0], S);

  auto S2 = vec_gather_u32<
    MixC3::Idx2,
    MixC2::Idx2,
    MixC1::Idx2,
    MixC0::Idx2>(&RJD_Te2[0], S);

  auto S3 = vec_gather_u32<
    MixC3::Idx3,
    MixC2::Idx3,
    MixC1::Idx3,
    MixC0::Idx3>(&RJD_Te3[0], S);

  return xor_(xor_(S0, S1), xor_(S2, S3));
}

void dump_state(Vec16 V) {
  alignas(Vec16) uint8_t V_[16];
  vec_store(V_, V);
  for (uint8_t C: V_) {
    printf("%02X", C);
  }
  printf("\n");
}
} // anonymous

void AESEncryptBlock(AESCtx const& C, uint8_t* Out, uint8_t const* In)
{
  Vec16 State = vec_loadu(In);
  State = xor_(State, vec_loadu(C.Keys[0]));
#ifndef NDEBUG
  dump_state(State);
#endif

  for (size_t R = 1; R < 10; ++R) {
    State = SRSBMixC_all(State);
    State = xor_(State, vec_loadu(C.Keys[R]));
#ifndef NDEBUG
    dump_state(State);
#endif
  }

  // TODO: make this cleaner!
  alignas(Vec16) uint8_t Tmp[16];
  vec_store(Tmp, State);
  for (size_t I = 0; I < 16; ++I) {
    Tmp[I] = RJD_SBOX[Tmp[SR[I]]];
  }
  State = xor_(vec_load(Tmp), vec_loadu(C.Keys[10]));
  vec_storeu(Out, State);
}

#else
namespace {
void dump_state(AESState const& S) {
  for (uint8_t C: S) {
    printf("%02X", C);
  }
  printf("\n");
}
}
void AESEncryptBlock(AESCtx const& C, uint8_t* Out, uint8_t const* In)
{
  alignas(uint32_t) AESState State;

  memcpy(&State[0], In, sizeof(State));
  State ^= C.Keys[0];
#ifndef NDEBUG
  dump_state(State);
#endif

  for (size_t R = 1; R < 10; ++R) {
    auto CurState = State;
    auto* S = &CurState[0];
    storeu_le<uint32_t>(&State[0],  SRSBMixC<MixC0>(S));
    storeu_le<uint32_t>(&State[4],  SRSBMixC<MixC1>(S));
    storeu_le<uint32_t>(&State[8],  SRSBMixC<MixC2>(S));
    storeu_le<uint32_t>(&State[12], SRSBMixC<MixC3>(S));
    State ^= C.Keys[R];
#ifndef NDEBUG
    dump_state(State);
#endif
  }

  // Last round is only sub bytes and shiftrows
  auto CurState = State;
  auto* S = &CurState[0];
  SRSB<MixC0>(&State[0],  S);
  SRSB<MixC1>(&State[4],  S);
  SRSB<MixC2>(&State[8],  S);
  SRSB<MixC3>(&State[12], S);
  State ^= C.Keys[10];
  memcpy(Out, &State[0], sizeof(State));
}
#endif

void AESDecryptBlock(AESCtx const& C, uint8_t* Out, uint8_t const* In)
{
#if 0
  AESState State;

  memcpy(&State.B[0], In, sizeof(State));
  State.B ^= C.Keys[10];

  auto CurState = State.B;

  InvSRSB< 0,13,10, 7>(&State.B[0], &CurState[0]);
  InvSRSB< 4, 1,14,11>(&State.B[4], &CurState[0]);
  InvSRSB< 8, 5, 6,15>(&State.B[8], &CurState[0]);
  InvSRSB<12, 9, 2, 3>(&State.B[12], &CurState[0]);

  for (size_t R = 9; R > 0; --R) {
    State.B ^= C.Keys[R];
    auto CurState = State.B;
    store_le<uint32_t>(&State.Cols[0], SRSBMixC< 0,13,10, 7>(&CurState[0]));
    store_le<uint32_t>(&State.Cols[1], SRSBMixC< 4, 1,14,11>(&CurState[0]));
    store_le<uint32_t>(&State.Cols[2], SRSBMixC< 8, 5, 6,15>(&CurState[0]));
    store_le<uint32_t>(&State.Cols[3], SRSBMixC<12, 9, 2, 3>(&CurState[0]));
  }

  State.B ^= C.Keys[0];
  memcpy(Out, &State.B[0], sizeof(State));
#endif
}

void AESKeyInvertExpand(uint8_t* Key, uint8_t const* RndKey, const size_t Rnd)
{
  if (Rnd == 0) {
    memcpy(Key, RndKey, sizeof(RoundKey));
    return;
  }

  if (Rnd > NR) {
    return;
  }

  RoundKey RKeys[NR+1];
  // This is basically the inverse of the algorithm above!
  RkBlock Tmp;

  memcpy(&RKeys[Rnd], RndKey, sizeof(RoundKey));

  for (ssize_t BI = 4*(Rnd)-1; BI >= 0; --BI) {
    Tmp = *GetRksBlock(RKeys, BI+3);

    if ((BI % 4) == 0) {
      // Rotate
      auto First = Tmp[0];
      Tmp[0] = Tmp[1];
      Tmp[1] = Tmp[2];
      Tmp[2] = Tmp[3];
      Tmp[3] = First;

      ApplySB(Tmp);

      Tmp[0] ^= RC_TBL[(BI+4)/4];
    }

    auto const& Next = *GetRksBlock(RKeys, BI+4);
    Tmp ^= Next;

    *GetRksBlock(RKeys, BI) = Tmp;
  }
  memcpy(Key, &RKeys[0], sizeof(RoundKey));
}

void AESKeyExpand(AESCtx& Ctx, uint8_t const* Key)
{
  RoundKey* RKeys = &Ctx.Keys[0];
  memcpy(&RKeys[0], Key, sizeof(RoundKey));
  RkBlock Tmp;

  // A "block" is a 4-byte stride of round keys
  for (size_t BI = 4; BI < 4*(NR+1); ++BI) {
    Tmp = *GetRksBlock(RKeys, BI-1);

    if (BI % 4 == 0) {
      // Rotate
      auto First = Tmp[0];
      Tmp[0] = Tmp[1];
      Tmp[1] = Tmp[2];
      Tmp[2] = Tmp[3];
      Tmp[3] = First;

      ApplySB(Tmp);

      Tmp[0] ^= RC_TBL[BI/4];
    }

    auto const& Prev = *GetRksBlock(RKeys, BI-4);
    Tmp ^= Prev;

    *GetRksBlock(RKeys, BI) = Tmp;
  }
}

} // aestoy
