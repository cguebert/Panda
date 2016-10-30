#pragma once

#include <panda/types/Point.h>

#include <set>

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

		panda::types::Point snapDelta() const;

		void prepareSnapTargets(object::ObjectRenderer* selectedRenderer);
		void updateSnapDelta(object::ObjectRenderer* selectedRenderer, const panda::types::Point& position);

	protected:
		GraphView& m_view;

		panda::types::Point m_snapDelta;
		std::set<float> m_snapTargetsY;
	};

//****************************************************************************//

	inline panda::types::Point SnapToObjects::snapDelta() const
	{ return m_snapDelta; }

} // namespace graphview
