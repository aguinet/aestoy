#include <cstdio>
#include <aestoy/aestoy.h>
#include <aestoy/tools.h>

using namespace aestoy;

int main(int argc, char** argv)
{
  if (argc < 2) {
    fprintf(stderr, "Usage: %s key_hex\n", argv[0]);
    fprintf(stderr, "This tool will dump the AES128 key schedule for the provided key.\n");
    return 1;
  }

  const char* KeyHex = argv[1];
  auto Key = fromHex(KeyHex);
  if (Key.size() != 16) {
    fprintf(stderr, "Size of key must be 16 bytes!\n");
    return 1;
  }
  
  AESCtx Ctx;
  AESKeyExpand(Ctx, &Key[0]);

  for (auto const& RK: Ctx.Keys) {
    dumpHex(stdout, std::begin(RK), std::end(RK));
    printf("\n");
  }

  return 0;
}
