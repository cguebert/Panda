#include <panda/object/ObjectFactory.h>
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
		, force(initData("force", "Force to apply to the particles"))
	{
		addInput(force);
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

//****************************************************************************//

class ParticleForceField_ForceInBox : public ParticleEffector
{
public:
	PANDA_CLASS(ParticleForceField_ForceInBox, ParticleEffector)

	ParticleForceField_ForceInBox(PandaDocument *doc)
		: ParticleEffector(doc)
		, force(initData("force", "Force to apply to the particles"))
		, box(initData("box", "Area in which particles are affected by the force"))
	{
		addInput(force);
		addInput(box);
	}

	void accumulateForces(Particles& particles)
	{
		const std::vector<Point>& forces = force.getValue();
		const std::vector<Rect>& boxes = box.getValue();

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
	Data< std::vector<Point> > force;
	Data< std::vector<Rect> > box;
};

int ParticleForceField_ForceInBoxClass = RegisterObject<ParticleForceField_ForceInBox>("Particles/Forces/Force in box")
		.setDescription("Apply a constant force to particles that are inside an area.");

//****************************************************************************//

class ParticleForceField_ForceInCircle : public ParticleEffector
{
public:
	PANDA_CLASS(ParticleForceField_ForceInCircle, ParticleEffector)

	ParticleForceField_ForceInCircle(PandaDocument *doc)
		: ParticleEffector(doc)
		, force(initData("force", "Force to apply to the particles"))
		, center(initData("center", "Center of the circle in which particles are affected by the force"))
		, radius(initData("radius", "Radius of the circle in which particles are affected by the force"))
	{
		addInput(force);
		addInput(center);
		addInput(radius);
	}

	void accumulateForces(Particles& particles)
	{
		const std::vector<Point>& forces = force.getValue();
		const std::vector<Point>& centers = center.getValue();
		const std::vector<float>& radiuses = radius.getValue();

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
				float r = radiuses[i%nbRadiuses];
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
	Data< std::vector<Point> > force, center;
	Data< std::vector<float> > radius;
};

int ParticleForceField_ForceInCircleClass = RegisterObject<ParticleForceField_ForceInCircle>("Particles/Forces/Force in circle")
		.setDescription("Apply a constant force to particles that are inside a circle.");

//****************************************************************************//

class ParticleForceField_Attraction : public ParticleEffector
{
public:
	PANDA_CLASS(ParticleForceField_Attraction, ParticleEffector)

	ParticleForceField_Attraction(PandaDocument *doc)
		: ParticleEffector(doc)
		, force(initData("force", "Maximum attraction force"))
		, center(initData("center", "Center of the circle in which particles are attracted"))
		, radius(initData("radius", "Radius of the circle in which particles are attracted"))
		, linear(initData(1, "linear", "How the force change depending on the distance to the center: true = linear law, false = quadratic"))
	{
		addInput(force);
		addInput(center);
		addInput(radius);
		addInput(linear);

		linear.setWidget("checkbox");
	}

	void accumulateForces(Particles& particles)
	{
		const std::vector<float>& forces = force.getValue();
		const std::vector<Point>& centers = center.getValue();
		const std::vector<float>& radiuses = radius.getValue();

		bool linearLaw = linear.getValue() != 0;
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
				const float& r = radiuses[i%nbRadiuses];
				float r2 = r * r;
				const float& f = forces[i%nbForces];
				for(auto& p : particles)
				{
					Point n = c - p.position;
					float d2 = n.norm2();
					if(d2 < r2)
					{
						float d = sqrt(d2);
						n.normalizeWithNorm(d);
						if(linearLaw)
							p.force += n * f * (1 - d / r);
						else
							p.force += n * f * (1 - d2 / r2);
					}
				}
			}
		}
	}

protected:
	Data< std::vector<float> > force;
	Data< std::vector<Point> > center;
	Data< std::vector<float> > radius;
	Data< int > linear;
};

int ParticleForceField_AttractionClass = RegisterObject<ParticleForceField_Attraction>("Particles/Forces/Attraction")
		.setDescription("Attract particles toward a point");


} // namespace Panda

