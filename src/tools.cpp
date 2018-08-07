#include <cstring>
#include <cstdio>
#include <vector>

#include <aestoy/tools.h>

namespace aestoy {

std::vector<uint8_t> fromHex(const char* Str)
{
  const size_t Len = strlen(Str)/2;
  std::vector<uint8_t> Ret;
  Ret.reserve(Len);
  for (size_t i = 0; i < Len; ++i) {
    uint8_t V;
    if (sscanf(&Str[2*i], "%02hhx", &V) != 1) {
      break;
    }
    Ret.push_back(V);
  }
  return Ret;
}

} // aestoy
