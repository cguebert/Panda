#include <panda/object/ObjectFactory.h>

#include <modules/particles/ParticleEffector.h>

namespace panda {

using types::Point;

class ParticleCollision_PenalityLines : public ParticleEffector
{
public:
	PANDA_CLASS(ParticleCollision_PenalityLines, ParticleEffector)

	ParticleCollision_PenalityLines(PandaDocument *doc)
		: ParticleEffector(doc)
		, lines(initData("lines", "The collision lines"))
		, repulsionForce(initData("force", "Maximum magnitude of the repulsion force"))
		, influenceDist(initData((PReal)20, "distance", "Threshold distance from the line below which particles are repulsed"))
		, bothSides(initData(1, "both sides", "If true, apply repulsion forces on both sides of the line"))
	{
		addInput(lines);
		addInput(repulsionForce);
		addInput(influenceDist);
		addInput(bothSides);

		bothSides.setWidget("checkbox");
	}

	virtual void accumulateForces(Particles& particles)
	{
		const std::vector<Point> pts = lines.getValue();
		const PReal maxForce = repulsionForce.getValue();
		PReal maxDist = influenceDist.getValue();
		maxDist *= maxDist;
		bool twoSides = bothSides.getValue() != 0;

		int nbPts = pts.size();
		for(int i=0; i<nbPts-1; ++i)
		{
			Point ptA = pts[i], ptB = pts[i+1];
			Point AB = ptB - ptA;
			Point normal = Point(AB.y, -AB.x).normalized();
			PReal ABn2 = AB.norm2();
			for(auto& particle : particles)
			{
				const Point& pos = particle.position;
				Point AP = pos - ptA;
				PReal r = (AP * AB) / ABn2;
				float sign = 1;
				if(AB.cross(AP) > 0)
				{
					if(!twoSides)
						continue;
					sign = -1;
				}
				if(r < 0 || r > 1)
					continue;
				Point pR = ptA + AB * r;
				PReal dist = (pos - pR).norm2();

				if(dist < maxDist)
				{
					PReal f = 1 - dist / maxDist;
					particle.force += sign * normal * f * maxForce;
				}
			}
		}
	}

protected:
	Data< std::vector<Point> > lines;
	Data< PReal> repulsionForce, influenceDist;
	Data< int > bothSides;
};

int ParticleCollision_PenalityLinesClass = RegisterObject<ParticleCollision_PenalityLines>("Particles/Collision/Lines")
		.setName("Collision lines")
		.setDescription("Apply repulsion forces to particles close to a line");


} // namespace Panda

