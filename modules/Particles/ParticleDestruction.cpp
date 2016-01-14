#include <panda/object/ObjectFactory.h>

#include <modules/particles/ParticleEffector.h>

#include <panda/types/Rect.h>
#include <panda/helper/algorithm.h>
#include <random>

namespace panda {

using types::Rect;

class ParticleDestruction_Age : public ParticleEffector
{
public:
	PANDA_CLASS(ParticleDestruction_Age, ParticleEffector)

	ParticleDestruction_Age(PandaDocument *doc)
		: ParticleEffector(doc)
		, maxAge(initData((PReal)1.0, "max age", "Remove particles older than this age"))
		, variation(initData((PReal)0.0, "variation", "Particles can randomly live longer by this amount"))
	{
		addInput(maxAge);
		addInput(variation);

		reset();
	}

	virtual void reset()
	{
		gen.seed(1);
		birth.clear();
	}

	virtual void onInitParticles(Particles& particles)
	{
		const PReal time = m_parentDocument->getAnimationTime();
		int oldNb = birth.size();
		int newNb = oldNb + particles.size();
		birth.resize(newNb);

		std::uniform_real_distribution<PReal> dist(0, variation.getValue());
		for(int i=oldNb; i<newNb; ++i)
			birth[i] = time - dist(gen);	// Giving them negative age based on the variation value
	}

	virtual Indices filterParticles(const Particles&)
	{
		Indices indices;

		const PReal time = m_parentDocument->getAnimationTime();
		int nb = birth.size();
		PReal age = maxAge.getValue();
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
	Data<PReal> maxAge, variation;
	std::vector<PReal> birth;
	std::mt19937 gen;
};

int ParticleDestruction_AgeClass = RegisterObject<ParticleDestruction_Age>("Particles/Destruction/Remove old particles")
		.setDescription("Remove particles older than a given age");

//****************************************************************************//

class ParticleDestruction_Index : public ParticleEffector
{
public:
	PANDA_CLASS(ParticleDestruction_Index, ParticleEffector)

	ParticleDestruction_Index(PandaDocument *doc)
		: ParticleEffector(doc)
		, indices(initData("indices", "Remove these particles"))
	{
		addInput(indices);
	}

	virtual Indices filterParticles(const Particles&)
	{
		Indices tmp;
		auto ind = indices.getValue();
		tmp.insert(ind.begin(), ind.end());

		return tmp;
	}

protected:
	Data< std::vector<int> > indices;
};

int ParticleDestruction_IndexClass = RegisterObject<ParticleDestruction_Index>("Particles/Destruction/Remove particles by index")
		.setDescription("Remove specific particles based on their index");

//****************************************************************************//

class ParticleDestruction_InRectangle : public ParticleEffector
{
public:
	PANDA_CLASS(ParticleDestruction_InRectangle, ParticleEffector)

	ParticleDestruction_InRectangle(PandaDocument *doc)
		: ParticleEffector(doc)
		, rectangle(initData("rectangle", "Remove particles that are in this rectangle"))
		, inverse(initData("inverse", "If true, remove particles that are outside the rectangle instead"))
	{
		addInput(rectangle);
		addInput(inverse);

		inverse.setWidget("checkbox");
	}

	virtual Indices filterParticles(const Particles& particles)
	{
		Indices tmp;
		int nb = particles.size();
		const std::vector<Rect>& rectangles = rectangle.getValue();
		for(const Rect& r : rectangles)
		{
			for(int i=0; i<nb; ++i)
			{
				if(r.contains(particles[i].position))
					tmp.insert(i);
			}
		}

		if(!inverse.getValue())
			return tmp;

		// Inverse selection
		Indices tmp2;
		int i=0;
		for(auto it=tmp.begin(); it!=tmp.end(); ++it)
		{
			int v = *it;
			while(i<v)
				tmp2.insert(i++);
			i = v+1;
		}
		while(i<nb)
			tmp2.insert(i++);

		return tmp2;
	}

protected:
	Data< std::vector<Rect> > rectangle;
	Data< int > inverse;
};

int ParticleDestruction_InRectangleClass = RegisterObject<ParticleDestruction_InRectangle>("Particles/Destruction/Remove particles in rectangle")
		.setDescription("Remove particles that are in an area");

} // namespace Panda

