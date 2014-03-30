#include <panda/ObjectFactory.h>
#include <panda/types/Rect.h>

#include <modules/particles/ParticleEffector.h>

namespace panda {

using types::Point;
using types::Rect;

class ParticleForceField_ConstantForce : public ParticleEffector
{
public:
	PANDA_CLASS(ParticleForceField_ConstantForce, ParticleEffector)

	ParticleForceField_ConstantForce(PandaDocument *doc)
		: ParticleEffector(doc)
		, force(initData(&force, "force", "Force to apply to the particles"))
	{
		addInput(&force);
	}

	void accumulateForces(Particles& particles)
	{
		Point f = force.getValue();
		for(auto& p : particles)
			p.force += f;
	}

protected:
	Data<Point> force;
};

int ParticleForceField_ConstantForceClass = RegisterObject<ParticleForceField_ConstantForce>("Particles/Forces/Constant force")
		.setDescription("Apply a constant force to the particles");

//*************************************************************************//

class ParticleForceField_ForceInBox : public ParticleEffector
{
public:
	PANDA_CLASS(ParticleForceField_ForceInBox, ParticleEffector)

	ParticleForceField_ForceInBox(PandaDocument *doc)
		: ParticleEffector(doc)
		, force(initData(&force, "force", "Force to apply to the particles"))
		, box(initData(&box, "box", "Area in which particles are affected by the force"))
	{
		addInput(&force);
		addInput(&box);
	}

	void accumulateForces(Particles& particles)
	{
		const QVector<Point>& forces = force.getValue();
		const QVector<Rect>& boxes = box.getValue();

		int nbForces = forces.size();
		int nbBoxes = boxes.size();
		if(nbForces && nbBoxes)
		{
			if(nbForces < nbBoxes) nbForces = 1;
			for(int i=0; i<nbBoxes; ++i)
			{
				const Rect& b = boxes[i];
				const Point& f = forces[i%nbForces];
				for(auto& p : particles)
				{
					if(b.contains(p.position))
						p.force += f;
				}
			}
		}
	}

protected:
	Data< QVector<Point> > force;
	Data< QVector<Rect> > box;
};

int ParticleForceField_ForceInBoxClass = RegisterObject<ParticleForceField_ForceInBox>("Particles/Forces/Force in box")
		.setDescription("Apply a constant force to particles that are inside an area.");

//*************************************************************************//

class ParticleForceField_ForceInCircle : public ParticleEffector
{
public:
	PANDA_CLASS(ParticleForceField_ForceInCircle, ParticleEffector)

	ParticleForceField_ForceInCircle(PandaDocument *doc)
		: ParticleEffector(doc)
		, force(initData(&force, "force", "Force to apply to the particles"))
		, center(initData(&center, "center", "Center of the circle in which particles are affected by the force"))
		, radius(initData(&radius, "radius", "Radius of the circle in which particles are affected by the force"))
	{
		addInput(&force);
		addInput(&center);
		addInput(&radius);
	}

	void accumulateForces(Particles& particles)
	{
		const QVector<Point>& forces = force.getValue();
		const QVector<Point>& centers = center.getValue();
		const QVector<PReal>& radiuses = radius.getValue();

		int nbForces = forces.size();
		int nbCenters = centers.size();
		int nbRadiuses = radiuses.size();
		if(nbForces && nbCenters && nbRadiuses)
		{
			if(nbForces < nbCenters) nbForces = 1;
			if(nbRadiuses < nbCenters) nbRadiuses = 1;
			for(int i=0; i<nbCenters; ++i)
			{
				const Point& c = centers[i];
				PReal r = radiuses[i%nbRadiuses];
				r *= r;
				const Point& f = forces[i%nbForces];
				for(auto& p : particles)
				{
					if((p.position - c).norm2() < r)
						p.force += f;
				}
			}
		}
	}

protected:
	Data< QVector<Point> > force, center;
	Data< QVector<PReal> > radius;
};

int ParticleForceField_ForceInCircleClass = RegisterObject<ParticleForceField_ForceInCircle>("Particles/Forces/Force in circle")
		.setDescription("Apply a constant force to particles that are inside a circle.");

} // namespace Panda

