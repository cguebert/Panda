#include "Random.h"
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
	gen.seed( (unsigned int)time(NULL) + (unsigned int)this );
	return (unsigned int)(random() * limit);
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
