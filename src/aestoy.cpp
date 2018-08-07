#include <array>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <aestoy/aestoy.h>
#include "aes_csts.h"

namespace aestoy {

namespace {

uint32_t load_le(uint8_t const* Ptr)
{
  uint32_t Ret;
  memcpy(&Ret, Ptr, sizeof(uint32_t));
#ifdef __BIG_ENDIAN__
  Ret = __builtin_bswap32(Ret);
#endif
  return Ret;
}

void store_le(uint8_t* Ptr, uint32_t const V)
{
#ifdef __BIG_ENDIAN__
  V = __builtin_bswap32(V);
#endif
  memcpy(Ptr, &V, sizeof(uint32_t));
}

struct State
{
  static constexpr size_t Nb = 4;
  static constexpr size_t BS = 4*Nb;

  uint8_t* column(size_t i)
  {
    assert(i < 4);
    return &state_[i*Nb];
  }

  void set_column(size_t i, uint32_t v)
  {
    store_le(column(i), v);
  }

  uint8_t* begin() { return std::begin(state_); }
  uint8_t* end() { return std::end(state_); }

  uint8_t const* begin() const { return std::begin(state_); }
  uint8_t const* end() const { return std::end(state_); }

  uint8_t& at(size_t i, size_t j)
  {
    assert(i < 4);
    assert(j < 4);
    return state_[i+j*4];
  }

  uint8_t at(size_t i, size_t j) const
  {
    assert(i < 4);
    assert(j < 4);
    return state_[i+j*4];
  }

  template <size_t by>
  void shift_row(size_t i)
  {
    if ((by == 0) || (by == Nb)) {
      return;
    }
    uint8_t srow[Nb];
    for (size_t j = 0; j < Nb; j++) {
      srow[j] = at(i, (j+by)%Nb);
    }
    for (size_t j = 0; j < Nb; j++) {
      at(i, j) = srow[j];
    }
  }

  uint8_t& operator[](size_t const i)
  {
    return state_[i];
  }

  uint8_t operator[](size_t const i) const
  {
    return state_[i];
  }

  std::array<uint8_t, BS> state_;
};

template <size_t N, class T>
__attribute__((always_inline)) T rol(T const v)
{
  return (v << N)|(v>>(sizeof(T)*8-N));
}

template <size_t N, class T>
__attribute__((always_inline)) T ror(T const v)
{
  return (v >> N)|(v<<(sizeof(T)*8-N));
}

template <class Container>
void ApplySB(Container& C)
{
  for (uint8_t& V: C) {
    V = RJD_SBOX[V];
  }
}

template <class Container>
void ApplyInvSB(Container& C)
{
  for (uint8_t& V: C) {
    V = RJD_SBOX_INV[V];
  }
}

void SubBytes(State& S) {
  ApplySB(S);
}

void InvSubBytes(State& S) {
  ApplyInvSB(S);
}

void AddRoundKey(State& S, RoundKey const& K) {
  for (size_t i = 0; i < 16; ++i) {
    S[i] ^= K[i];
  }
}

void ShiftRows(State& S)
{
  S.shift_row<1>(1);
  S.shift_row<2>(2);
  S.shift_row<3>(3);
}

void InvShiftRows(State& S)
{
  S.shift_row<3>(1);
  S.shift_row<2>(2);
  S.shift_row<1>(3);
}

void MixColumns(State& S)
{
  for(size_t i = 0; i < 4; i++) {
    uint32_t C =
      RJD_MC[S.at(0,i)] ^
      rol<8>(RJD_MC[S.at(1,i)]) ^
      rol<16>(RJD_MC[S.at(2,i)]) ^
      rol<24>(RJD_MC[S.at(3,i)]);
    S.set_column(i, C);
  }
}

void InvMixColumns(State& S)
{
  for(size_t i = 0; i < 4; i++) {
    uint32_t C =
      RJD_INVMC[S.at(0,i)] ^
      rol<8>(RJD_INVMC[S.at(1,i)]) ^
      rol<16>(RJD_INVMC[S.at(2,i)]) ^
      rol<24>(RJD_INVMC[S.at(3,i)]);
    S.set_column(i, C);
  }
}

void AESEncrypt(State& S, RoundKey const* Keys)
{
  AddRoundKey(S, Keys[0]);
  for (size_t R = 1; R < NR; ++R) {
    SubBytes(S);
    ShiftRows(S);
    MixColumns(S);
    AddRoundKey(S, Keys[R]);
  }
  SubBytes(S);
  ShiftRows(S);
  AddRoundKey(S, Keys[NR]);
}

void AESDecrypt(State& S, RoundKey const* Keys)
{
  AddRoundKey(S, Keys[NR]);
  InvShiftRows(S);
  InvSubBytes(S);
  for (size_t R = NR-1; R > 0; --R) {
    AddRoundKey(S, Keys[R]);
    InvMixColumns(S);
    InvShiftRows(S);
    InvSubBytes(S);
  }
  AddRoundKey(S, Keys[0]);
}

using RkBlock = std::array<uint8_t, 4>;
RkBlock* GetRksBlock(RoundKey* RKeys, size_t n)
{
  size_t idx = n*4;
  return (RkBlock*) &RKeys[idx/16][idx%16];
}

} // anonymous

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
    for (size_t i = 0; i < sizeof(Tmp); ++i) {
      Tmp[i] ^= Prev[i];
    }
    *GetRksBlock(RKeys, BI) = Tmp;
  }
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
    for (size_t i = 0; i < sizeof(Tmp); ++i) {
      Tmp[i] ^= Next[i];
    }

    *GetRksBlock(RKeys, BI) = Tmp;
  }
  memcpy(Key, &RKeys[0], sizeof(RoundKey));
}

void AESEncryptBlock(AESCtx const& C, uint8_t* Out, uint8_t const* In)
{
  State S;
  memcpy(&S, In, sizeof(S));
  AESEncrypt(S, &C.Keys[0]);
  memcpy(Out, &S, sizeof(S));
}

void AESDecryptBlock(AESCtx const& C, uint8_t* Out, uint8_t const* In)
{
  State S;
  memcpy(&S, In, sizeof(State));
  AESDecrypt(S, &C.Keys[0]);
  memcpy(Out, &S, sizeof(State));
}

} // aestoy
