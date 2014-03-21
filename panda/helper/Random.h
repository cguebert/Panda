#ifndef HELPER_RANDOM_H
#define HELPER_RANDOM_H

#include <panda/helper/system/Config.h>
#include <random>

namespace panda
{

namespace helper
{

class RandomGenerator
{
public:
	void seed(unsigned int seed);
	static unsigned int getRandomSeed(unsigned int limit);

	PReal random();
	PReal random(PReal min, PReal max);

	int randomInt(int min, int max);

protected:
	std::mt19937 gen;
};

} // namespace helper

} // namespace panda

#endif // HELPER_RANDOM_H
