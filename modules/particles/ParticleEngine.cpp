#include <panda/Dockable.h>
#include <panda/PandaDocument.h>
#include <panda/ObjectFactory.h>
#include <QPointF>
#include <QList>
#include <helper/Random.h>
#include <algorithm>

#define _USE_MATH_DEFINES
#include <math.h>

#include "ParticleEffector.h"

namespace panda {

class ParticleEngine : public DockObject
{
public:
	PANDA_CLASS(ParticleEngine, DockObject)

	ParticleEngine(PandaDocument *doc)
		: DockObject(doc)
		, document(doc)
		, positions(initData(&positions, "positions", "List of the particles' position"))
		, velocities(initData(&velocities, "velocities", "List of the particles' velocity"))
		, accelerations(initData(&accelerations, "accelerations", "List of the particles' acceleration"))
	{
		addOutput(&positions);
		addOutput(&velocities);
		addOutput(&accelerations);
	}

	void updateEffectors()
	{
		effectors.clear();
		DockablesIterator iter = getDockablesIterator();
		while(iter.hasNext())
		{
			ParticleEffector* effector = dynamic_cast<ParticleEffector*>(iter.next());
			if(effector)
				effectors.append(effector);
		}
	}

	virtual bool accepts(DockableObject* dockable) const
	{
		return (dynamic_cast<ParticleEffector*>(dockable) !=  nullptr);
	}

	void removeParticles()
	{
		QSet<int> indices;
		for(auto effector : effectors)
			indices.unite(effector->filterParticles());

		for(auto effector : effectors)
			effector->onRemoveParticles(indices);

		for(int i : indices)	// filter the particles we want to remove
			particles[i].index = -1;
		particles.erase(std::remove_if(particles.begin(), particles.end(), [](Particle& p){ return p.index == -1;}), particles.end());
	}

	void createNewParticles()
	{
		QVector<Particle> newParticles;
		int nbNew = 0;
		for(auto effector : effectors)
		{
			auto list = effector->createParticles();
			int nb = list.size();
			if(nb)
			{
				newParticles.resize(nbNew + nb);
				qCopy(list.begin(), list.end(), newParticles.begin()+nbNew);
				nbNew += nb;
			}
		}

		for(auto effector : effectors)
			effector->onInitParticles(newParticles);

		int prevNb = particles.size();
		particles.resize(prevNb + nbNew);
		qCopy(newParticles.begin(), newParticles.end(), particles.begin()+prevNb);

		// update the particles' indices
		int i=0;
		for(auto& p : particles)
			p.index = i++;
	}

	void update()
	{
		updateEffectors();

		removeParticles();
		createNewParticles();

		for(auto& p : particles)
			p.force = QPointF(0,0);

		for(auto effector : effectors)
			effector->accumulateForces(particles);

		for(auto effector : effectors)
			effector->postUpdate(particles);

		// move the particles
		qreal dt = document->getTimeStep();
		for(auto& p : particles)
		{
			QPointF oldVel = p.velocity;
			p.acceleration = p.force;
			p.force = QPointF(0,0);
			p.velocity += p.acceleration * dt;
			p.position += (p.velocity + oldVel) * 0.5 * dt;
		}

		// copy particles values to the output Datas
		auto pos = positions.getAccessor();
		auto vel = velocities.getAccessor();
		auto acc = accelerations.getAccessor();

		int nb = particles.size();
		pos.resize(nb);
		vel.resize(nb);
		acc.resize(nb);

		int i=0;
		for(auto& p : particles)
		{
			pos[i] = p.position;
			vel[i] = p.velocity;
			acc[i] = p.acceleration;
		}

		this->cleanDirty();
	}

protected:
	PandaDocument* document;
	QVector<ParticleEffector*> effectors;
	QVector<Particle> particles;
	Data< QVector<QPointF> > positions, velocities, accelerations;
};

int ParticleEngineClass = RegisterObject("Particles/Particle engine").setClass<ParticleEngine>().setName("Particle engine").setDescription("Animate a set of points based on physical properties and optional modifier objects");

} // namespace Panda
