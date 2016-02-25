#include <modules/Box2D/World.h>

#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/Mesh.h>
#include <panda/types/Path.h>

namespace panda 
{

using types::Mesh;
using types::Path;
using types::Point;
using MeshsList = std::vector<Mesh>;
using PathsList = std::vector<Path>;
using PointsList = std::vector<Point>;

class Box2D_Particles : public Box2DDockable
{
public:
	PANDA_CLASS(Box2D_Particles, Box2DDockable)

		Box2D_Particles(PandaDocument *doc)
		: Box2DDockable(doc)
		, m_input(initData("input", "Position of the particles to add to the simulation"))
		, m_output(initData("output", "Position of the particles during the simulation"))
		, m_radius(initData(5, "radius", "Radius of the particles"))
	{
		addInput(m_input);
		addInput(m_radius);

		addOutput(m_output);
	}

	void reset() override
	{
		m_output.setValue(m_input.getValue());
	}

	void update() override
	{
		if (isInStep())
			return;

		reset();
	}

	void initBox2D(WorldData& data) override
	{
		m_particleSystem = nullptr;
		const auto& points = m_input.getValue();
		const auto radius = m_radius.getValue();

		m_output.setValue(points);
		if (points.empty() || radius < 1)
			return;
		
		const auto scaling = data.scaling();

		b2ParticleSystemDef sysDef;
		sysDef.radius = radius / scaling;
		m_particleSystem = data.world().CreateParticleSystem(&sysDef);

		b2ParticleDef pdef;
		for (const auto& pt : points)
		{
			auto pos = pt / scaling;
			pdef.position.Set(pos.x, pos.y);
			m_particleSystem->CreateParticle(pdef);
		}
	}

	void postStepBox2D(WorldData& data) override
	{
		if (!m_particleSystem)
			return;

		const int nb = m_particleSystem->GetParticleCount();
		auto positions = m_particleSystem->GetPositionBuffer();
		const auto scaling = data.scaling();

		auto output = m_output.getAccessor();
		for (int i = 0; i < nb; ++i)
			output[i] = b2Panda::convert(positions[i]) * scaling;
	}

protected:
	Data<PointsList> m_input, m_output;
	Data<float> m_radius;

	b2ParticleSystem* m_particleSystem = nullptr;
};

int Box2D_ParticlesClass = RegisterObject<Box2D_Particles>("Box2D/Particles").setDescription("Create some Box2D particles");

//****************************************************************************//

} // namespace Panda
