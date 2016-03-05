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

class Box2D_StaticPoly : public Box2DDockable
{
public:
	PANDA_CLASS(Box2D_StaticPoly, Box2DDockable)

	Box2D_StaticPoly(PandaDocument *doc)
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

int Box2D_StaticPolyClass = RegisterObject<Box2D_StaticPoly>("Box2D/Static polygon").setDescription("Create a Box2D static object using a polygon");

//****************************************************************************//

class Box2D_StaticMesh : public Box2DDockable
{
public:
	PANDA_CLASS(Box2D_StaticMesh, Box2DDockable)

		Box2D_StaticMesh(PandaDocument *doc)
		: Box2DDockable(doc)
		, m_input(initData("input", "Static bodies to add to the simulation"))
	{
		addInput(m_input);
	}

	inline b2PolygonShape convertTriangle(const Mesh::SeqPoints& pts, const Mesh::Triangle& tri, float scaling)
	{
		b2PolygonShape shape;
		b2Vec2 vertices[3];
		for (int i = 0; i < 3; ++i)
			vertices[i] = b2Panda::convert(pts[tri[i]] / scaling);
		shape.Set(vertices, 3);
		return shape;
	}

	void initBox2D(WorldData& data) override
	{
		const auto& meshes = m_input.getValue();
		if (meshes.empty())
			return;

		b2BodyDef bodyDef;
		const auto scaling = data.scaling();
		auto body = data.world().CreateBody(&bodyDef);

		for (const auto& mesh : meshes)
		{
			const auto& pts = mesh.getPoints();
			for (const auto& tri : mesh.getTriangles())
			{
				auto shape = convertTriangle(pts, tri, scaling);
				body->CreateFixture(&shape, 0.f);
			}
		}
	}

protected:
	Data<MeshsList> m_input;
};

int Box2D_StaticMeshClass = RegisterObject<Box2D_StaticMesh>("Box2D/Static mesh").setDescription("Create a Box2D static object using a mesh");

//****************************************************************************//

class Box2D_DynamicPoly : public Box2DDockable
{
public:
	PANDA_CLASS(Box2D_DynamicPoly, Box2DDockable)

	Box2D_DynamicPoly(PandaDocument *doc)
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
				path.points.push_back(b2Panda::convert(pt));
			output.push_back(path); // At the wrong place now, but it will be moved at the end of the timestep anyway
		}
	}

	void postStepBox2D(WorldData& data) override
	{
		auto outputList = m_output.getAccessor();
		int nb = m_bodies.size();
		const auto scaling = data.scaling();

		for (int i = 0; i < nb; ++i)
		{
			const auto& body = m_bodies[i];
			const auto bPtr = body.first;
			auto& output = outputList[i].points;
			
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

int Box2D_DynamicPolyClass = RegisterObject<Box2D_DynamicPoly>("Box2D/Dynamic polygon").setDescription("Create a Box2D dynamic object using a polygon");

//****************************************************************************//

class Box2D_DynamicCircle : public Box2DDockable
{
public:
	PANDA_CLASS(Box2D_DynamicCircle, Box2DDockable)

	Box2D_DynamicCircle(PandaDocument *doc)
		: Box2DDockable(doc)
		, m_inputCenter(initData("input center", "Center of the circles to add to the simulation"))
		, m_inputRadius(initData("input radius", "Radius of the circles to add to the simulation"))
		, m_outputCenter(initData("output center", "Position of the circles during the simulation"))
		, m_outputRotation(initData("output rotation", "Rotation of the circles during the simulation"))
	{
		addInput(m_inputCenter);
		addInput(m_inputRadius);
		addOutput(m_outputCenter);
		addOutput(m_outputRotation);
	}

	void reset() override
	{
		m_outputCenter.setValue(m_inputCenter.getValue());
		m_outputRotation.getAccessor().wref().assign(m_inputCenter.getValue().size(), 0);
	}

	void update() override
	{
		if (isInStep())
			return;

		reset();
	}

	void initBox2D(WorldData& data) override
	{
		const auto& centerList = m_inputCenter.getValue();
		const auto& radiusList = m_inputRadius.getValue();

		m_bodies.clear();

		int nbC = centerList.size();
		m_outputCenter.setValue(m_inputCenter.getValue());
		m_outputRotation.getAccessor().wref().assign(nbC, 0);
		
		if (centerList.empty() || radiusList.empty())
			return;

		int nbR = radiusList.size();
		if (nbR < nbC) nbR = 1;

		auto& world = data.world();
		const auto scaling = data.scaling();
		for (int i = 0; i < nbC; ++i)
		{
			const auto& center = centerList[i];
			b2BodyDef bodyDef;
			bodyDef.type = b2_dynamicBody;

			bodyDef.position.Set(center.x / scaling, center.y / scaling);
			b2Body* body = world.CreateBody(&bodyDef);

			b2CircleShape circleShape;
			circleShape.m_radius = radiusList[i % nbR] / scaling;

			b2FixtureDef fixtureDef;
			fixtureDef.shape = &circleShape;
			fixtureDef.density = 1.0f;
			body->CreateFixture(&fixtureDef);

			m_bodies.push_back(body);
		}
	}

	void postStepBox2D(WorldData& data) override
	{
		auto centerList = m_outputCenter.getAccessor();
		auto rotationList = m_outputRotation.getAccessor();
		int nb = m_bodies.size();

		const auto scaling = data.scaling();
		for (int i = 0; i < nb; ++i)
		{
			const auto& body = m_bodies[i];
			centerList[i] = b2Panda::convert(body->GetPosition()) * scaling;
			rotationList[i] = body->GetAngle();
		}
	}

protected:
	Data<PointsList> m_inputCenter, m_outputCenter;
	Data<std::vector<float>> m_inputRadius, m_outputRotation;

	std::vector<b2Body*> m_bodies;
};

int Box2D_DynamicCircleClass = RegisterObject<Box2D_DynamicCircle>("Box2D/Dynamic circle").setDescription("Create a Box2D dynamic object using a circle");

} // namespace Panda
