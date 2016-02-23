#include <panda/object/Dockable.h>

#include <panda/types/Path.h>

#include <Box2D/Box2D.h>

namespace panda 
{

class WorldData
{
public:
	WorldData(b2Vec2 gravity, float scaling)
		: m_world(gravity)
		, m_scaling(scaling)
	{}

	inline b2World& world()
	{ return m_world; }

	inline float scaling() const
	{ return m_scaling; }

	b2PolygonShape createStaticShape(const types::Path& path);

	struct DynamicShape
	{
		b2Vec2 center;
		b2PolygonShape shape;
		std::vector<b2Vec2> path;
	};
	DynamicShape createDynamicShape(const types::Path& path);

private:
	b2World m_world;
	float m_scaling;
};

class Box2DDockable : public DockableObject
{
public:
	Box2DDockable(PandaDocument* doc)
		: DockableObject(doc)
	{}

	virtual void initBox2D(WorldData& data) {};
	virtual void preStepBox2D(WorldData& data) {};
	virtual void postStepBox2D(WorldData& data) {};
};

namespace b2Panda // Useful methods to convert panda types to Box2D and back
{
	inline b2Vec2 convert(const panda::types::Point& pt)
	{ return b2Vec2(pt.x, pt.y); }

	inline panda::types::Point convert(const b2Vec2& pt)
	{ return panda::types::Point(pt.x, pt.y); }

	types::Point computeCenterOfMass(const types::Path& path);
}

} // namespace Panda
