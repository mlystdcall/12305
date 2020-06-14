#include "includes.hpp"

std::pair<int,int> Hash :: hash( const char *str ) const {
	int h1 = 0, h2 = 0;
	for( ; *str; ++str ) {
		h1 = int((1LL * h1 * BASE + *str) % MOD1);
		h2 = int((1LL * h2 * BASE + *str) % MOD2);
	}
	return std::make_pair(h1, h2);
}
