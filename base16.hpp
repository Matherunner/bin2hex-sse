#ifndef BASE16_HPP
#define BASE16_HPP

#include <cstdlib>

void base16_enc_sse4(const char *in, size_t insiz, char *out, bool uppercase);

#endif