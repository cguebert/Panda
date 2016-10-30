#pragma once

#include <panda/types/Point.h>

#include <set>

namespace panda
{

namespace graphview
{
	namespace object {
		class ObjectRenderer;
	}

	class GraphView;

	class PANDA_CORE_API SnapToObjects
	{
	public:
		SnapToObjects(GraphView& view);

		types::Point snapDelta() const;

		void prepareSnapTargets(object::ObjectRenderer* selectedRenderer);
		void updateSnapDelta(object::ObjectRenderer* selectedRenderer, const types::Point& position);

	protected:
		GraphView& m_view;

		types::Point m_snapDelta;
		std::set<float> m_snapTargetsY;
	};

//****************************************************************************//

	inline types::Point SnapToObjects::snapDelta() const
	{ return m_snapDelta; }

} // namespace graphview

} // namespace panda
