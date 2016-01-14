#ifndef PARTICLEEFFECTOR_H
#define PARTICLEEFFECTOR_H

#include <panda/object/Dockable.h>
#include <panda/PandaDocument.h>
#include <modules/particles/Particle.h>
#include <set>

namespace panda {

class ParticleEffector : public DockableObject
{
public:
	PANDA_CLASS(ParticleEffector, DockableObject)

	typedef std::vector<Particle> Particles;
	typedef std::set<int> Indices;

	ParticleEffector(PandaDocument *doc)
		: DockableObject(doc)
	{
	}

	virtual Particles createParticles()		// If we want to create new particles
	{ return Particles(); }
	virtual void onInitParticles(Particles&) {}	// After particles have been created (can modify them)

	virtual Indices filterParticles(const Particles&) // Set of indices of particles to be removed
	{ return Indices(); }
	virtual void onRemoveParticles(const Particles&, const Indices&) {} // Before removing particles, we have the list of indices of the particles that will be removed

	virtual void accumulateForces(Particles&) {}// Main function to be derived to change the movement of a particle
	virtual void postUpdate(Particles&) {}		// For treatment after the particle has been moved
};

} // namespace Panda

#endif
