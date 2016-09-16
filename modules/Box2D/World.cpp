#include <modules/Box2D/World.h>

#include <panda/object/ObjectFactory.h>

#include <memory>

namespace panda 
{

using types::Path;
using types::Point;

class Box2D_World : public DockObject
{
public:
	PANDA_CLASS(Box2D_World, DockObject)

	Box2D_World(PandaDocument* doc)
		: DockObject(doc)
		, m_gravity(initData(Point(0, 10.f), "gravity", "The world gravity vector"))
		, m_timestep(initData(1/60.f, "timestep", "The amount of time to simulate"))
		, m_scaling(initData(50, "scaling", "The number of pixels in 1 meter in the simulation"))
	{
		addInput(m_gravity);
		addInput(m_timestep);
		addInput(m_scaling);
	}

	bool accepts(DockableObject* dockable) const override
	{
		return (dynamic_cast<Box2DDockable*>(dockable) != nullptr);
	}

	void reset() override
	{
		m_firstStep = true;
	}

	void beginStep() override
	{
		helper::ScopedEvent event("Box2D Step", this);

		if (m_firstStep)
		{
			auto gravity = b2Panda::convert(m_gravity.getValue());
			auto scaling = m_scaling.getValue();
			m_timestepValue = m_timestep.getValue();

			m_worldData = std::make_shared<WorldData>(gravity, scaling);
			m_firstStep = false;

			// Get the objects list
			m_modifiers.clear();
			for (auto object : getDockedObjects())
			{
				auto mod = dynamic_cast<Box2DDockable*>(object);
				if (mod)
					m_modifiers.push_back(mod);
			}

			for (auto mod : m_modifiers)
				mod->initBox2D(*m_worldData);
		}
		else
		{
			const int velocityIterations = 6;
			const int positionIterations = 2;

			auto& data = *m_worldData;
			for (auto mod : m_modifiers)
				mod->preStepBox2D(data);

			data.world().Step(m_timestepValue, velocityIterations, positionIterations);

			for (auto mod : m_modifiers)
				mod->postStepBox2D(data);
		}
	}

protected:
	Data<Point> m_gravity;
	Data<float> m_timestep, m_scaling;

	bool m_firstStep = true;
	float m_timestepValue;
	std::vector<Box2DDockable*> m_modifiers;
	
	std::shared_ptr<WorldData> m_worldData;
};

int Box2D_WorldClass = RegisterObject<Box2D_World>("Box2D/Box2D World").setDescription("Create a Box2D simulation");

//****************************************************************************//

namespace b2Panda
{
	
	types::Point computeCenterOfMass(const types::Path& path)
	{
		if (path.points.empty())
			return Point();
		Point c;
		for (const auto& pt : path.points)
			c += pt;

		int nb = path.points.size();
		if (path.points.front() == path.points.back())
		{
			c -= path.points.back();
			nb -= 1;
		}
		c /= static_cast<float>(nb);
		return c;
	}

}

//****************************************************************************//

b2PolygonShape WorldData::createStaticShape(const types::Path& path)
{
	std::vector<b2Vec2> tmp;
	tmp.reserve(path.points.size());

	for (const auto& pt : path.points)
		tmp.push_back(b2Panda::convert(pt / m_scaling));

	b2PolygonShape shape;
	shape.Set(tmp.data(), tmp.size());

	return shape;
}

WorldData::DynamicShape WorldData::createDynamicShape(const types::Path& path)
{
	auto center = b2Panda::computeCenterOfMass(path);
	auto nbPts = path.points.size();

	DynamicShape dyn;
	dyn.path.reserve(nbPts);

	for (const auto& pt : path.points)
		dyn.path.push_back(b2Panda::convert((pt - center) / m_scaling));

	std::vector<b2Vec2> tmp = dyn.path;
	if (tmp.front() == tmp.back())
		tmp.pop_back();

	dyn.shape.Set(tmp.data(), tmp.size());
	dyn.center = b2Panda::convert(center / m_scaling);
	return dyn;
}

//****************************************************************************//

panda::ModuleHandle box2DModule = REGISTER_MODULE
		.setDescription("Components to create physical simulations using the Box2D engine")
		.setLicense("GPL")
		.setVersion("1.0");


} // namespace Panda
