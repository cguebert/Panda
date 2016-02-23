#include <modules/Box2D/World.h>

#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/Path.h>

namespace panda 
{

using types::Path;
using PathsList = std::vector<Path>;

class Box2D_Static : public Box2DDockable
{
public:
	PANDA_CLASS(Box2D_Static, Box2DDockable)

	Box2D_Static(PandaDocument *doc)
		: Box2DDockable(doc)
		, m_input(initData("input", "Static bodies to add to the simulation"))
	{
		addInput(m_input);
	}

	void initBox2D(WorldData& data) override
	{
		const auto& paths = m_input.getValue();
		if (paths.empty())
			return;

		b2BodyDef bodyDef;
		auto body = data.world().CreateBody(&bodyDef);
		
		for (const auto& path : paths)
		{
			auto shape = data.createStaticShape(path);
			body->CreateFixture(&shape, 0.f);
		}
	}

protected:
	Data<PathsList> m_input;
};

int Box2D_StaticClass = RegisterObject<Box2D_Static>("Box2D/Static body").setDescription("Create a Box2D static object");

//****************************************************************************//

class Box2D_Dynamic : public Box2DDockable
{
public:
	PANDA_CLASS(Box2D_Dynamic, Box2DDockable)

	Box2D_Dynamic(PandaDocument *doc)
		: Box2DDockable(doc)
		, m_input(initData("input", "Dynamic bodies to add to the simulation"))
		, m_output(initData("output", "Bodies moved by the simulation"))
	{
		addInput(m_input);
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
		const auto& paths = m_input.getValue();
		auto output = m_output.getAccessor();
		output.clear();
		m_bodies.clear();
		
		if (paths.empty())
			return;

		int nb = paths.size();
		output.reserve(nb);

		auto& world = data.world();
		
		for (const auto& path : paths)
		{
			auto dynamicShape = data.createDynamicShape(path);
			
			b2BodyDef bodyDef;
			bodyDef.type = b2_dynamicBody;

			auto pos = dynamicShape.center;
			bodyDef.position.Set(pos.x, pos.y);
			b2Body* body = world.CreateBody(&bodyDef);

			b2FixtureDef fixtureDef;
			fixtureDef.shape = &dynamicShape.shape;
			fixtureDef.density = 1.0f;
			body->CreateFixture(&fixtureDef);

			m_bodies.emplace_back(body, dynamicShape.path);

			Path path;
			for (const auto& pt : dynamicShape.path)
				path.push_back(b2Panda::convert(pt));
			output.push_back(path); // At the wrong place now, but it will be moved at the end of the timestep anyway
		}
	}

	void postStepBox2D(WorldData& data) override
	{
		auto outputList = m_output.getAccessor();
		int nb = m_bodies.size();

		for (int i = 0; i < nb; ++i)
		{
			const auto& body = m_bodies[i];
			const auto bPtr = body.first;
			auto& output = outputList[i];

			const auto scaling = data.scaling();
			const auto& path = body.second;
			int nbPts = path.size();
			for (int j = 0; j < nbPts; ++j)
				output[j] = b2Panda::convert(bPtr->GetWorldPoint(path[j])) * scaling;
		}
	}

protected:
	Data<PathsList> m_input, m_output;

	using b2Path = std::vector<b2Vec2>;
	using DynamicBodyPair = std::pair<b2Body*, b2Path>;
	std::vector<DynamicBodyPair> m_bodies;
};

int Box2D_DynamicClass = RegisterObject<Box2D_Dynamic>("Box2D/Dynamic body").setDescription("Create a Box2D dynamic object");

} // namespace Panda
