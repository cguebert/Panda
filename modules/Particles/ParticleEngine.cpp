#include <panda/object/Dockable.h>
#include <panda/PandaDocument.h>
#include <panda/object/ObjectFactory.h>

#include <modules/particles/ParticleEngine.h>
#include <modules/particles/ParticleEffector.h>

#include <QList>
#include <algorithm>
#include <panda/helper/algorithm.h>

#include <cmath>

#ifdef PANDA_LOG_EVENTS
#include <panda/UpdateLogger.h>
#endif

namespace panda {

using types::Point;

ParticleEngine::ParticleEngine(PandaDocument *doc)
	: DockObject(doc)
	, positions(initData("positions", "List of the particles' position"))
	, velocities(initData("velocities", "List of the particles' velocity"))
	, accelerations(initData("accelerations", "List of the particles' acceleration"))
	, prevTime(0.0)
{
	addOutput(positions);
	addOutput(velocities);
	addOutput(accelerations);

	BaseData* docTime = doc->getData("time");
	if(docTime)
		addInput(*docTime);
}

void ParticleEngine::reset()
{
	prevTime = 0.0;

	particles.clear();
	positions.getAccessor().clear();
	velocities.getAccessor().clear();
	accelerations.getAccessor().clear();
}

void ParticleEngine::updateEffectors()
{
	effectors.clear();
	for(auto dockable : getDockedObjects())
	{
		ParticleEffector* effector = dynamic_cast<ParticleEffector*>(dockable);
		if(effector)
			effectors.push_back(effector);
	}
}

bool ParticleEngine::accepts(DockableObject* dockable) const
{
	return (dynamic_cast<ParticleEffector*>(dockable) !=  nullptr);
}

void ParticleEngine::removeParticles()
{
	ParticleEffector::Indices indices;
	for(auto effector : effectors)
	{
		auto temp = effector->filterParticles(particles);
		indices.insert(std::begin(temp), std::end(temp));
	}

	for(auto effector : effectors)
		effector->onRemoveParticles(particles, indices);

	helper::removeIndices(particles, indices);
}

void ParticleEngine::createNewParticles()
{
	std::vector<Particle> newParticles;
	int nbNew = 0;
	for(auto effector : effectors)
	{
		auto list = effector->createParticles();
		int nb = list.size();
		if(nb)
		{
			helper::concatenate(newParticles, list);
			nbNew += nb;
		}
	}

	for(auto effector : effectors)
		effector->onInitParticles(newParticles);

	helper::concatenate(particles, newParticles);

	// update the particles' indices
	int i=0;
	for(auto& p : particles)
		p.index = i++;
}

void ParticleEngine::update()
{
	PReal time = m_parentDocument->getAnimationTime();
	if(prevTime == time)
		return;

	prevTime = time;

	updateEffectors();

	removeParticles();
	createNewParticles();

	for(auto& p : particles)
		p.force = Point(0,0);

	for(auto effector : effectors)
		effector->accumulateForces(particles);

	for(auto effector : effectors)
		effector->postUpdate(particles);

	// move the particles
	qreal dt = m_parentDocument->getTimeStep();
	for(auto& p : particles)
	{
		Point oldVel = p.velocity;
		p.acceleration = p.force;
		p.force = Point(0,0);
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

	for(int i=0; i<nb; ++i)
	{
		const Particle& p = particles[i];
		pos[i] = p.position;
		vel[i] = p.velocity;
		acc[i] = p.acceleration;
	}

	cleanDirty();
}

int ParticleEngineClass = RegisterObject<ParticleEngine>("Particles/Particle engine").setDescription("Animate a set of points based on physical properties and optional modifier objects");

ModuleHandle particlesModule = REGISTER_MODULE
		.setDescription("Particle engine and effectors")
		.setLicense("GPL")
		.setVersion("1.0");

} // namespace Panda
