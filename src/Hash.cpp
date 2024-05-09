#include "Hash.h"

#define A_PRIME 54059
#define ANOTHER_PRIME 76963
#define FIRSTH 37 /* also prime */

unsigned sf::Hash::SimpleStringHash(const char* s)
{
	unsigned h = FIRSTH;
	while (*s) {
		h = (h * A_PRIME) ^ (s[0] * ANOTHER_PRIME);
		s++;
	}
	return h;
}