#ifndef PARTICLEEFFECTOR_H
#define PARTICLEEFFECTOR_H

#include <panda/Dockable.h>
#include <panda/PandaDocument.h>
#include "Particle.h"
#include <QSet>

namespace panda {

class ParticleEffector : public DockableObject
{
public:
	PANDA_CLASS(ParticleEffector, DockableObject)

	ParticleEffector(PandaDocument *doc)
		: DockableObject(doc)
	{
	}

	virtual QVector<Particle> createParticles() {}		// If we want to create new particles
	virtual void onInitParticles(QVector<Particle>&) {}	// After particles have been created (can modify them)

	virtual QSet<int> filterParticles() {}				// Set of indices of particles to be removed
	virtual void onRemoveParticles(const QSet<int>&) {}	// Before removing particles, we have the list of indices of the particles that will be removed

	virtual void accumulateForces(QVector<Particle>&) {}// Main function to be derived to change the movement of a particle
	virtual void postUpdate(QVector<Particle>&) {}		// For treatment after the particle has been moved
};

} // namespace Panda

#endif
