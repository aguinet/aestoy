#ifndef AESTOY_TOOLS_H
#define AESTOY_TOOLS_H

#include <cstdio>
#include <cstdint>
#include <vector>

namespace aestoy {

std::vector<uint8_t> fromHex(const char* Str);

template <class Iterator>
void dumpHex(FILE* F, Iterator Begin, Iterator End)
{
  for (auto It = Begin; It != End; ++It) {
    fprintf(F, "%02X", *It);
  }
}

} // aestoy

#endif
