#pragma once

#include <panda/types/Point.h>

namespace graphview
{
	namespace object {
		class ObjectRenderer;
	}

	class GraphView;

	class SnapToObjects
	{
	public:
		SnapToObjects(GraphView& view);

		void prepareSnapTargets(object::ObjectRenderer* selectedRenderer);
		void computeSnapDelta(object::ObjectRenderer* selectedRenderer, const panda::types::Point& position);

	protected:
		GraphView& m_view;

		std::set<float> m_snapTargetsY;
		panda::types::Point m_snapDelta;
	};

} // namespace graphview
