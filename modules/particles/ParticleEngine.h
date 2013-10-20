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
	Q_OBJECT
public:
	PANDA_CLASS(ParticleEngine, DockObject)

	ParticleEngine(PandaDocument *doc);

	virtual void reset();
	virtual bool accepts(DockableObject* dockable) const;
	virtual void update();

	void updateEffectors();

	void removeParticles();
	void createNewParticles();

private slots:
	void timeChanged();

protected:
	QVector<ParticleEffector*> effectors;
	QVector<Particle> particles;
	Data< QVector<QPointF> > positions, velocities, accelerations;
	double prevTime;
};

} // namespace Panda

#endif // PARTICLEENGINE_H