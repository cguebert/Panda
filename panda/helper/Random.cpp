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
	gen.seed( static_cast<unsigned int>(time(nullptr)) + reinterpret_cast<uintptr_t>(this) );
	return static_cast<unsigned int>(random() * limit);
}

double RandomGenerator::random()
{
	return dist(gen);
}

double RandomGenerator::random(double min, double max)
{
	return min + random() * (max - min);
}

} // namespace helper

} // namespace panda