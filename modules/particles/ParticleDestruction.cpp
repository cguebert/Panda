#include <panda/ObjectFactory.h>

#include <modules/particles/ParticleEffector.h>

#include <panda/helper/Algorithm.h>
#include <random>

namespace panda {

class ParticleDestruction_Age : public ParticleEffector
{
public:
	PANDA_CLASS(ParticleDestruction_Age, ParticleEffector)

	ParticleDestruction_Age(PandaDocument *doc)
		: ParticleEffector(doc)
		, maxAge(initData(&maxAge, 1.0, "max age", "Remove particles older than this age"))
		, variation(initData(&variation, 0.0, "variation", "Particles can randomly live longer by this amount"))
	{
		addInput(&maxAge);
		addInput(&variation);

		reset();
	}

	virtual void reset()
	{
		gen.seed(1);
		birth.clear();
	}

	virtual void onInitParticles(Particles& particles)
	{
		const double time = parentDocument->getAnimationTime();
		int oldNb = birth.size();
		int newNb = oldNb + particles.size();
		birth.resize(newNb);

		std::uniform_real_distribution<double> dist(0, variation.getValue());
		for(int i=oldNb; i<newNb; ++i)
			birth[i] = time - dist(gen);	// Giving them negative age based on the variation value
	}

	virtual Indices filterParticles(const Particles&)
	{
		Indices indices;

		const double time = parentDocument->getAnimationTime();
		int nb = birth.size();
		double age = maxAge.getValue();
		for(int i=0; i<nb; ++i)
		{
			if(time >= birth[i] + age)
				indices.insert(i);
		}

		return indices;
	}

	virtual void onRemoveParticles(const Particles&, const Indices& indices)
	{
		helper::removeIndices(birth, indices);
	}

protected:
	Data<double> maxAge, variation;
	QVector<double> birth;
	std::mt19937 gen;
};

int ParticleDestruction_AgeClass = RegisterObject<ParticleDestruction_Age>("Particles/Destruction/Remove old particles")
		.setDescription("Remove particles older than a given age");

//*************************************************************************//

class ParticleDestruction_Index : public ParticleEffector
{
public:
	PANDA_CLASS(ParticleDestruction_Index, ParticleEffector)

	ParticleDestruction_Index(PandaDocument *doc)
		: ParticleEffector(doc)
		, indices(initData(&indices, "indices", "Remove these particles"))
	{
		addInput(&indices);
	}

	virtual Indices filterParticles(const Particles&)
	{
		Indices tmp;
		auto ind = indices.getValue();
		tmp.insert(ind.begin(), ind.end());

		return tmp;
	}

protected:
	Data< QVector<int> > indices;
};

int ParticleDestruction_IndexClass = RegisterObject<ParticleDestruction_Index>("Particles/Destruction/Remove particles by index")
		.setDescription("Remove specific particles based on their index");

} // namespace Panda

