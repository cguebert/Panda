#ifndef HELPER_RANDOM_H
#define HELPER_RANDOM_H

#include <panda/core.h>
#include <random>

namespace panda
{

namespace helper
{

class PANDA_CORE_API RandomGenerator
{
public:
	void seed(unsigned int seed);
	static unsigned int getRandomSeed(unsigned int limit);

	PReal random();
	float random(float min, float max);
	double random(double min, double max);

	int randomInt(int min, int max);

private:
	std::mt19937 gen;
};

} // namespace helper

} // namespace panda

#endif // HELPER_RANDOM_H
