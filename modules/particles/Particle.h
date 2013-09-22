#ifndef PARTICLE_H
#define PARTICLE_H

#include <QPointF>

namespace panda {

class Particle
{
public:
	int index;
	QPointF position, velocity, acceleration, force;
};

} // namespace Panda

#endif
