#ifndef PARTICLEENGINE_H
#define PARTICLEENGINE_H

#include <panda/object/Dockable.h>
#include <modules/particles/Particle.h>

namespace panda {

class PandaDocument;
class ParticleEffector;

class ParticleEngine : public DockObject
{
public:
	PANDA_CLASS(ParticleEngine, DockObject)

	ParticleEngine(PandaDocument *doc);

	virtual void reset();
	virtual bool accepts(DockableObject* dockable) const;
	virtual void update();

	void updateEffectors();

	void removeParticles();
	void createNewParticles();

protected:
	std::vector<ParticleEffector*> effectors;
	std::vector<Particle> particles;
	Data< std::vector<types::Point> > positions, velocities, accelerations;
	float prevTime;
};

} // namespace Panda

#endif // PARTICLEENGINE_H
