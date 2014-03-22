#ifndef PARTICLE_H
#define PARTICLE_H

#include <panda/types/Point.h>

namespace panda {

class Particle
{
public:
	Particle()
		: index(-1)
	{ }

	int index;
	types::Point position, velocity, acceleration, force;
};

} // namespace Panda

#endif
