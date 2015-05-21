#include "base16.hpp"
#include <x86intrin.h>

void base16_enc_sse4(const char *in, size_t insiz, char *out, bool uppercase)
{
	static const uint8_t HEX_TABLE_UPPERCASE[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
		'A', 'B', 'C', 'D', 'E', 'F'
	};
	static const uint8_t HEX_TABLE_LOWERCASE[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
		'a', 'b', 'c', 'd', 'e', 'f'
	};
	const __m128i CONST_0_CHR = _mm_set1_epi8('0');
	const __m128i CONST_9 = _mm_set1_epi8(9);
	const __m128i OFFSET = uppercase ? _mm_set1_epi8('A' - 9 - 1)
		: _mm_set1_epi8('a' - 9 - 1);
	const __m128i AND4BITS = _mm_set1_epi8(0xf);

	const __m128i *pinvec = (const __m128i *)in;
	__m128i *poutvec = (__m128i *)out;
	size_t leftover = insiz % 16;
	insiz /= 16;

	while (insiz--) {
		// invec = [b0, b1, ..., b15]
		__m128i invec = _mm_loadu_si128(pinvec++);

		// masked1 = [b0 & 0xf, b1 & 0xf, ...]
		// masked2 = [b0 >> 4, b1 >> 4, ...]
		__m128i masked1 = _mm_and_si128(invec, AND4BITS);
		__m128i masked2 = _mm_srli_epi64(invec, 4);
		masked2 = _mm_and_si128(masked2, AND4BITS);

		// return 0xff corresponding to the elements > 9, or 0x00 otherwise
		__m128i cmpmask1 = _mm_cmpgt_epi8(masked1, CONST_9);
		__m128i cmpmask2 = _mm_cmpgt_epi8(masked2, CONST_9);

		// add '0' or the offset depending on the masks
		__m128i add1 = _mm_blendv_epi8(CONST_0_CHR, OFFSET, cmpmask1);
		__m128i add2 = _mm_blendv_epi8(CONST_0_CHR, OFFSET, cmpmask2);
		masked1 = _mm_add_epi8(masked1, add1);
		masked2 = _mm_add_epi8(masked2, add2);

		// interleave masked1 and masked2 bytes
		__m128i res1 = _mm_unpacklo_epi8(masked2, masked1);
		__m128i res2 = _mm_unpackhi_epi8(masked2, masked1);
		_mm_storeu_si128(poutvec++, res1);
		_mm_storeu_si128(poutvec++, res2);
	}

	const uint8_t *pinval = (const uint8_t *)pinvec;
	uint8_t *poutval = (uint8_t *)poutvec;
	const uint8_t *TABLE = uppercase ? HEX_TABLE_UPPERCASE
		: HEX_TABLE_LOWERCASE;

	while (leftover--) {
		uint8_t inval = *pinval++;
		*poutval++ = TABLE[inval >> 4];
		*poutval++ = TABLE[inval & 0xf];
	}
}