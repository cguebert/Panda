#include <panda/helper/Random.h>
#include <time.h>

namespace panda
{

namespace helper
{

void RandomGenerator::seed(unsigned int seed)
{
	gen.seed(seed);
}

unsigned int RandomGenerator::seedRandom(unsigned int limit)
{
	std::uniform_int_distribution<int> dist(0, limit);
	std::random_device rnd;
	return dist(rnd);
}

double RandomGenerator::random()
{
	std::uniform_real_distribution<double> dist(0, 1);
	return dist(gen);
}

double RandomGenerator::random(double min, double max)
{
	std::uniform_real_distribution<double> dist(std::min(min, max), std::max(min, max));
	return dist(gen);
}

int RandomGenerator::randomInt(int min, int max)
{
	std::uniform_int_distribution<int> dist(std::min(min, max), std::max(min, max));
	return dist(gen);
}

} // namespace helper

} // namespace panda
