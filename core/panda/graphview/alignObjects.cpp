#include <panda/graphview/alignObjects.h>
#include <panda/graphview/GraphView.h>
#include <panda/graphview/ObjectsSelection.h>
#include <panda/graphview/ObjectRenderersList.h>
#include <panda/graphview/object/ObjectRenderer.h>
#include <panda/command/MoveObjectCommand.h>
#include <panda/document/PandaDocument.h>

namespace panda
{

using types::Point;
using types::Rect;

namespace graphview
{

void alignHorizontallyCenter(GraphView& view)
{
	float sum = 0;
	auto objRndList = view.objectRenderers().get(view.selection().get());
	for(auto objRnd : objRndList)
	{
		Point pos = objRnd->getPosition();
		Rect area = objRnd->getVisualArea();
		sum += pos.x + area.width() / 2;
	}

	float center = sum / objRndList.size();

	auto& undoStack = view.document()->getUndoStack();
	auto moveMacro = undoStack.beginMacro("center on vertical axis");

	for(auto objRnd : objRndList)
	{
		Point pos = objRnd->getPosition();
		Rect area = objRnd->getVisualArea();
		Point delta = Point(center - pos.x - area.width() / 2, 0);
		if(!delta.isNull())
			undoStack.push(std::make_shared<MoveObjectCommand>(objRnd->getObject(), delta));
	}
}

void alignHorizontallyLeft(GraphView& view)
{
	float left = std::numeric_limits<float>::max();
	auto objRndList = view.objectRenderers().get(view.selection().get());
	for(auto objRnd : objRndList)
	{
		Point pos = objRnd->getPosition();
		left = std::min(pos.x, left);
	}

	auto& undoStack = view.document()->getUndoStack();
	auto moveMacro = undoStack.beginMacro("align left edges");

	for(auto objRnd : objRndList)
	{
		Point delta = Point(left - objRnd->getPosition().x, 0);
		if(!delta.isNull())
			undoStack.push(std::make_shared<MoveObjectCommand>(objRnd->getObject(), delta));
	}
}

void alignHorizontallyRight(GraphView& view)
{
	float right = std::numeric_limits<float>::lowest();
	auto objRndList = view.objectRenderers().get(view.selection().get());
	for(auto objRnd : objRndList)
	{
		Point pos = objRnd->getPosition();
		Rect area = objRnd->getVisualArea();
		right = std::max(pos.x + area.width(), right);
	}

	auto& undoStack = view.document()->getUndoStack();
	auto moveMacro = undoStack.beginMacro("align right edges");

	for(auto objRnd : objRndList)
	{
		Point pos = objRnd->getPosition();
		Rect area = objRnd->getVisualArea();
		Point delta = Point(right - pos.x - area.width(), 0);
		if(!delta.isNull())
			undoStack.push(std::make_shared<MoveObjectCommand>(objRnd->getObject(), delta));
	}
}

void alignVerticallyCenter(GraphView& view)
{
	float sum = 0;
	auto objRndList = view.objectRenderers().get(view.selection().get());
	for(auto objRnd : objRndList)
	{
		Point pos = objRnd->getPosition();
		Rect area = objRnd->getVisualArea();
		sum += pos.y + area.height() / 2;
	}

	float center = sum / objRndList.size();

	auto& undoStack = view.document()->getUndoStack();
	auto moveMacro = undoStack.beginMacro("center on horizontal axis");

	for(auto objRnd : objRndList)
	{
		Point pos = objRnd->getPosition();
		Rect area = objRnd->getVisualArea();
		Point delta = Point(0, center - pos.y - area.height() / 2);
		if(!delta.isNull())
			undoStack.push(std::make_shared<MoveObjectCommand>(objRnd->getObject(), delta));
	}
}

void alignVerticallyTop(GraphView& view)
{
	float top = std::numeric_limits<float>::max();
	auto objRndList = view.objectRenderers().get(view.selection().get());
	for(auto objRnd : objRndList)
	{
		Point pos = objRnd->getPosition();
		top = std::min(pos.y, top);
	}

	auto& undoStack = view.document()->getUndoStack();
	auto moveMacro = undoStack.beginMacro("align top edges");

	for(auto objRnd : objRndList)
	{
		Point delta = Point(0, top - objRnd->getPosition().y);
		if(!delta.isNull())
			undoStack.push(std::make_shared<MoveObjectCommand>(objRnd->getObject(), delta));
	}
}

void alignVerticallyBottom(GraphView& view)
{
	float bottom = std::numeric_limits<float>::lowest();
	auto objRndList = view.objectRenderers().get(view.selection().get());
	for(auto objRnd : objRndList)
	{
		Point pos = objRnd->getPosition();
		Rect area = objRnd->getVisualArea();
		bottom = std::max(pos.y + area.height(), bottom);
	}

	auto& undoStack = view.document()->getUndoStack();
	auto moveMacro = undoStack.beginMacro("align bottom edges");

	for(auto objRnd : objRndList)
	{
		Point pos = objRnd->getPosition();
		Rect area = objRnd->getVisualArea();
		Point delta = Point(0, bottom - pos.y - area.height());
		if(!delta.isNull())
			undoStack.push(std::make_shared<MoveObjectCommand>(objRnd->getObject(), delta));
	}
}

void distributeHorizontallyCenter(GraphView& view)
{
	auto objRndList = view.objectRenderers().get(view.selection().get());
	int nb = objRndList.size();
	if (nb <= 2)
		return;

	using PosOds = std::pair<float, object::ObjectRenderer*>;
	std::vector<PosOds> positions;
	for(auto objRnd : objRndList)
	{
		Point pos = objRnd->getPosition();
		Rect area = objRnd->getVisualArea();
		positions.emplace_back(pos.x + area.width() / 2, objRnd);
	}

	std::sort(positions.begin(), positions.end(), [](const PosOds& lhs, const PosOds& rhs) { return lhs.first < rhs.first; });

	float left = positions.front().first;
	float right = positions.back().first;
	float step = (right - left) / (nb - 1);

	auto& undoStack = view.document()->getUndoStack();
	auto moveMacro = undoStack.beginMacro("distribute centers horizontally");

	for (int i = 1; i < nb; ++i)
	{
		const auto& posPair = positions[i];
		left += step;
		auto objRnd = posPair.second;
		Point pos = objRnd->getPosition();
		Rect area = objRnd->getVisualArea();
		Point delta = Point(left - pos.x - area.width() / 2, 0);
		if(!delta.isNull())
			undoStack.push(std::make_shared<MoveObjectCommand>(objRnd->getObject(), delta));
	}
}

void distributeHorizontallyGaps(GraphView& view)
{
	auto objRndList = view.objectRenderers().get(view.selection().get());
	int nb = objRndList.size();
	if (nb <= 2)
		return;

	using PosOds = std::pair<float, object::ObjectRenderer*>;
	std::vector<PosOds> positions;
	for(auto objRnd : objRndList)
	{
		Point pos = objRnd->getPosition();
		Rect area = objRnd->getVisualArea();
		positions.emplace_back(pos.x, objRnd);
	}

	std::sort(positions.begin(), positions.end(), [](const PosOds& lhs, const PosOds& rhs) { return lhs.first < rhs.first; });

	float left = positions.front().first + positions.front().second->getVisualArea().width();
	float right = positions.back().first;
	
	// Compute total gap
	float width = 0;
	for (int i = 1; i < nb - 1; ++i)
		width += positions[i].second->getVisualArea().width();

	float step = (right - left - width) / (nb - 1);

	auto& undoStack = view.document()->getUndoStack();
	auto moveMacro = undoStack.beginMacro("distribute gaps horizontally");

	for (int i = 1; i < nb; ++i)
	{
		const auto& posPair = positions[i];
		left += step;
		auto objRnd = posPair.second;
		Point pos = objRnd->getPosition();
		Rect area = objRnd->getVisualArea();
		Point delta = Point(left - pos.x, 0);
		if(!delta.isNull())
			undoStack.push(std::make_shared<MoveObjectCommand>(objRnd->getObject(), delta));

		left += area.width();
	}
}

void distributeHorizontallyLeft(GraphView& view)
{
	auto objRndList = view.objectRenderers().get(view.selection().get());
	int nb = objRndList.size();
	if (nb <= 2)
		return;

	using PosOds = std::pair<float, object::ObjectRenderer*>;
	std::vector<PosOds> positions;
	for(auto objRnd : objRndList)
	{
		Point pos = objRnd->getPosition();
		Rect area = objRnd->getVisualArea();
		positions.emplace_back(pos.x, objRnd);
	}

	std::sort(positions.begin(), positions.end(), [](const PosOds& lhs, const PosOds& rhs) { return lhs.first < rhs.first; });

	float left = positions.front().first;
	float right = positions.back().first;
	float step = (right - left) / (nb - 1);

	auto& undoStack = view.document()->getUndoStack();
	auto moveMacro = undoStack.beginMacro("distribute left edges");

	for (int i = 1; i < nb; ++i)
	{
		const auto& posPair = positions[i];
		left += step;
		auto objRnd = posPair.second;
		Point pos = objRnd->getPosition();
		Rect area = objRnd->getVisualArea();
		Point delta = Point(left - pos.x, 0);
		if(!delta.isNull())
			undoStack.push(std::make_shared<MoveObjectCommand>(objRnd->getObject(), delta));
	}
}

void distributeHorizontallyRight(GraphView& view)
{
	auto objRndList = view.objectRenderers().get(view.selection().get());
	int nb = objRndList.size();
	if (nb <= 2)
		return;

	using PosOds = std::pair<float, object::ObjectRenderer*>;
	std::vector<PosOds> positions;
	for(auto objRnd : objRndList)
	{
		Point pos = objRnd->getPosition();
		Rect area = objRnd->getVisualArea();
		positions.emplace_back(pos.x + area.width(), objRnd);
	}

	std::sort(positions.begin(), positions.end(), [](const PosOds& lhs, const PosOds& rhs) { return lhs.first < rhs.first; });

	float left = positions.front().first;
	float right = positions.back().first;
	float step = (right - left) / (nb - 1);

	auto& undoStack = view.document()->getUndoStack();
	auto moveMacro = undoStack.beginMacro("distribute right edges");

	for (int i = 1; i < nb; ++i)
	{
		const auto& posPair = positions[i];
		left += step;
		auto objRnd = posPair.second;
		Point pos = objRnd->getPosition();
		Rect area = objRnd->getVisualArea();
		Point delta = Point(left - pos.x - area.width(), 0);
		if(!delta.isNull())
			undoStack.push(std::make_shared<MoveObjectCommand>(objRnd->getObject(), delta));
	}
}

void distributeVerticallyCenter(GraphView& view)
{
	auto objRndList = view.objectRenderers().get(view.selection().get());
	int nb = objRndList.size();
	if (nb <= 2)
		return;

	using PosOds = std::pair<float, object::ObjectRenderer*>;
	std::vector<PosOds> positions;
	for(auto objRnd : objRndList)
	{
		Point pos = objRnd->getPosition();
		Rect area = objRnd->getVisualArea();
		positions.emplace_back(pos.y + area.height() / 2, objRnd);
	}

	std::sort(positions.begin(), positions.end(), [](const PosOds& lhs, const PosOds& rhs) { return lhs.first < rhs.first; });

	float top = positions.front().first;
	float bottom = positions.back().first;
	float step = (bottom - top) / (nb - 1);

	auto& undoStack = view.document()->getUndoStack();
	auto moveMacro = undoStack.beginMacro("distribute centers vertically");

	for (int i = 1; i < nb; ++i)
	{
		const auto& posPair = positions[i];
		top += step;
		auto objRnd = posPair.second;
		Point pos = objRnd->getPosition();
		Rect area = objRnd->getVisualArea();
		Point delta = Point(0, top - pos.y - area.height() / 2);
		if(!delta.isNull())
			undoStack.push(std::make_shared<MoveObjectCommand>(objRnd->getObject(), delta));
	}
}

void distributeVerticallyGaps(GraphView& view)
{
	auto objRndList = view.objectRenderers().get(view.selection().get());
	int nb = objRndList.size();
	if (nb <= 2)
		return;

	using PosOds = std::pair<float, object::ObjectRenderer*>;
	std::vector<PosOds> positions;
	for(auto objRnd : objRndList)
	{
		Point pos = objRnd->getPosition();
		Rect area = objRnd->getVisualArea();
		positions.emplace_back(pos.y, objRnd);
	}

	std::sort(positions.begin(), positions.end(), [](const PosOds& lhs, const PosOds& rhs) { return lhs.first < rhs.first; });

	float top = positions.front().first + positions.front().second->getVisualArea().height();
	float bottom = positions.back().first;
	
	// Compute total gap
	float height = 0;
	for (int i = 1; i < nb - 1; ++i)
		height += positions[i].second->getVisualArea().height();

	float step = (bottom - top - height) / (nb - 1);

	auto& undoStack = view.document()->getUndoStack();
	auto moveMacro = undoStack.beginMacro("distribute gaps vertically");

	for (int i = 1; i < nb; ++i)
	{
		const auto& posPair = positions[i];
		top += step;
		auto objRnd = posPair.second;
		Point pos = objRnd->getPosition();
		Rect area = objRnd->getVisualArea();
		Point delta = Point(0, top - pos.y);
		if(!delta.isNull())
			undoStack.push(std::make_shared<MoveObjectCommand>(objRnd->getObject(), delta));

		top += area.height();
	}
}

void distributeVerticallyTop(GraphView& view)
{
	auto objRndList = view.objectRenderers().get(view.selection().get());
	int nb = objRndList.size();
	if (nb <= 2)
		return;

	using PosOds = std::pair<float, object::ObjectRenderer*>;
	std::vector<PosOds> positions;
	for(auto objRnd : objRndList)
	{
		Point pos = objRnd->getPosition();
		Rect area = objRnd->getVisualArea();
		positions.emplace_back(pos.y, objRnd);
	}

	std::sort(positions.begin(), positions.end(), [](const PosOds& lhs, const PosOds& rhs) { return lhs.first < rhs.first; });

	float top = positions.front().first;
	float bottom = positions.back().first;
	float step = (bottom - top) / (nb - 1);

	auto& undoStack = view.document()->getUndoStack();
	auto moveMacro = undoStack.beginMacro("distribute top edges");

	for (int i = 1; i < nb; ++i)
	{
		const auto& posPair = positions[i];
		top += step;
		auto objRnd = posPair.second;
		Point pos = objRnd->getPosition();
		Rect area = objRnd->getVisualArea();
		Point delta = Point(0, top - pos.y);
		if(!delta.isNull())
			undoStack.push(std::make_shared<MoveObjectCommand>(objRnd->getObject(), delta));
	}
}

void distributeVerticallyBottom(GraphView& view)
{
	auto objRndList = view.objectRenderers().get(view.selection().get());
	int nb = objRndList.size();
	if (nb <= 2)
		return;

	using PosOds = std::pair<float, object::ObjectRenderer*>;
	std::vector<PosOds> positions;
	for(auto objRnd : objRndList)
	{
		Point pos = objRnd->getPosition();
		Rect area = objRnd->getVisualArea();
		positions.emplace_back(pos.y + area.height(), objRnd);
	}

	std::sort(positions.begin(), positions.end(), [](const PosOds& lhs, const PosOds& rhs) { return lhs.first < rhs.first; });

	float top = positions.front().first;
	float bottom = positions.back().first;
	float step = (bottom - top) / (nb - 1);

	auto& undoStack = view.document()->getUndoStack();
	auto moveMacro = undoStack.beginMacro("distribute bottom edges");

	for (int i = 1; i < nb; ++i)
	{
		const auto& posPair = positions[i];
		top += step;
		auto objRnd = posPair.second;
		Point pos = objRnd->getPosition();
		Rect area = objRnd->getVisualArea();
		Point delta = Point(0, top - pos.y - area.height());
		if(!delta.isNull())
			undoStack.push(std::make_shared<MoveObjectCommand>(objRnd->getObject(), delta));
	}
}

} // namespace graphview

} // namespace panda
