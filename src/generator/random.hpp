///
/// hash algorithm for randomness that doesn't need good distribution
///
#ifndef RANDOM_HPP
#define RANDOM_HPP

#include <cstdint>

inline unsigned int hash(unsigned int x)
{
	x = (x ^ 61) ^ (x >> 16);
	x = x + (x << 3);
	x = x ^ (x >> 4);
	x = x * 0x27d4eb2d;
	x = x ^ (x >> 15);
	return x;
}
inline int ihash(int x)
{
	return hash(x) & INT32_MAX;
}
inline int ihash(int x, int y)
{
	return hash(x ^ ihash(y)) & INT32_MAX;
}
inline int ihash(int x, int y, int z)
{
	return hash(x ^ ihash(y) ^ ihash(z)) & INT32_MAX;
}

inline float randf(int x)
{
	return ihash(x) / (float)INT32_MAX;
}
inline float randf(int x, int y)
{
	return randf(x ^ ihash(y));
}
inline float randf(int x, int y, int z)
{
	return randf(x ^ ihash(y) ^ ihash(z));
}

inline int rand2d(int x, int z)
{
	return ihash(x + 5953) ^ ihash(z + 7639);
}

inline unsigned int wang_hash(int key)
{
  key += ~(key << 15);
  key ^=  (key >> 10);
  key +=  (key << 3);
  key ^=  (key >> 6);
  key +=  (key << 11);
  key ^=  (key >> 16);
  return key;
}
inline unsigned int wang_seed2d(int x, int z, int seed)
{
  return wang_hash(x ^ wang_hash(z ^ seed));
}

#endif
