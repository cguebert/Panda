#include <panda/ObjectFactory.h>

#include <modules/particles/ParticleEffector.h>

namespace panda {

using types::Point;

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


} // namespace Panda

