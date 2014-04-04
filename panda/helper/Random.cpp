#include <panda/helper/Random.h>
#include <algorithm>

namespace panda
{

namespace helper
{

void RandomGenerator::seed(unsigned int seed)
{
	gen.seed(seed+1);
}

unsigned int RandomGenerator::getRandomSeed(unsigned int limit)
{
	std::uniform_int_distribution<int> dist(0, limit);
	std::random_device rnd;
	return dist(rnd);
}

PReal RandomGenerator::random()
{
	std::uniform_real_distribution<PReal> dist(0, 1);
	return dist(gen);
}

float RandomGenerator::random(float min, float max)
{
	std::uniform_real_distribution<float> dist(std::min(min, max), std::max(min, max));
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
