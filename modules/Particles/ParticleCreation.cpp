#include <panda/object/ObjectFactory.h>

#include <modules/particles/ParticleEffector.h>

#include <algorithm>

namespace panda {

using types::Point;

class ParticleCreation_FromLists : public ParticleEffector
{
public:
	PANDA_CLASS(ParticleCreation_FromLists, ParticleEffector)

	ParticleCreation_FromLists(PandaDocument *doc)
		: ParticleEffector(doc)
		, position(initData("position", "Where to place new particles"))
		, velocity(initData("velocity", "Initial velocity of the particles"))
	{
		addInput(position);
		addInput(velocity);
	}

	virtual Particles createParticles()
	{
		Particles newParticles;
		const auto& posList = position.getValue();
		const auto& velList = velocity.getValue();

		int nbPos = posList.size();
		int nbVel = velList.size();

		if(nbPos)
		{
			if(nbPos < nbVel) nbPos = 1;
			if(nbVel > 0 && nbVel < nbPos) nbVel = 1;
			int nb = std::max(nbPos, nbVel);
		
			newParticles.resize(nb);
			for(int i=0; i<nb; ++i)
			{
				newParticles[i].position = posList[i % nbPos];
				if(nbVel)
					newParticles[i].velocity = velList[i % nbVel];
			}
		}

		return newParticles;
	}

protected:
	Data< std::vector<Point> > position, velocity;
};

int ParticleCreation_FromListsClass = RegisterObject<ParticleCreation_FromLists>("Particles/Creation/Particles from lists")
		.setDescription("Create new particles based on lists of positions and velocities");


} // namespace Panda

