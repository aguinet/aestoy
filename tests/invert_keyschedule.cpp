#include <cstdio>
#include <cstring>
#include <random>

#include <aestoy/aestoy.h>
#include <aestoy/tools.h>

using namespace aestoy;

static void RandKey(uint8_t* Key)
{
  static std::random_device rd;
  for (size_t i = 0; i < sizeof(RoundKey); ++i) {
    Key[i] = rd();
  }
}

int main()
{
  AESCtx Ctx;
  uint8_t Key[sizeof(RoundKey)];
  uint8_t Out[sizeof(RoundKey)];
  for (unsigned i = 0; i < 64; ++i) {
    RandKey(Key);
    AESKeyExpand(Ctx, Key);
    for (unsigned R = 0; R <= NR; ++R) {
      AESKeyInvertExpand(Out, &Ctx.Keys[R][0], R);
      if (memcmp(Out, Key, sizeof(Key) != 0)) {
        fprintf(stderr, "Invalid key inversion for key ");
        dumpHex(stderr, std::begin(Key), std::end(Key));
        fprintf(stderr, " at round %u\n", R);
        return 1;
      }
    }
  }
  return 0;
}
