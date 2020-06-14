#ifndef HASH_HPP
#define HASH_HPP

class Hash {
public:
	enum {
		BASE = 19260817,
		MOD1 = 998244353,
		MOD2 = 1000000007
	};
	
	std::pair<int,int> hash( const char *str ) const;
};

#endif // HASH_HPP
