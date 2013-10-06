#ifndef HELPER_RANDOM_H
#define HELPER_RANDOM_H

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_01.hpp>

namespace panda
{

namespace helper
{

class RandomGenerator
{
public:
	void seed(unsigned int seed);
	unsigned int seedRandom(unsigned int limit);
	double random();
	double random(double min, double max);

protected:
	boost::random::mt19937 gen;
	boost::random::uniform_01<double> dist;
};

} // namespace helper

} // namespace panda

#endif // HELPER_RANDOM_H
