#ifndef HELPER_RANDOM_H
#define HELPER_RANDOM_H

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

	double random();
	double random(double min, double max);

	int randomInt(int min, int max);

protected:
	std::mt19937 gen;
};

} // namespace helper

} // namespace panda

#endif // HELPER_RANDOM_H
