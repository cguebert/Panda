#ifndef PARTICLEENGINE_H
#define PARTICLEENGINE_H

#include <panda/Dockable.h>
#include <modules/particles/Particle.h>

#include <QVector>

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
	QVector<ParticleEffector*> effectors;
	QVector<Particle> particles;
	Data< QVector<types::Point> > positions, velocities, accelerations;
	PReal prevTime;
};

} // namespace Panda

#endif // PARTICLEENGINE_H
