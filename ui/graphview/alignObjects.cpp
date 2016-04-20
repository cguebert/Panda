#include <ui/graphview/alignObjects.h>
#include <ui/graphview/GraphView.h>
#include <ui/graphview/ObjectsSelection.h>
#include <ui/drawstruct/ObjectDrawStruct.h>
#include <ui/command/MoveObjectCommand.h>

#include <panda/PandaDocument.h>

using panda::types::Point;
using panda::types::Rect;

void alignHorizontallyCenter(GraphView* view)
{
	float sum = 0;
	auto odsList = view->getObjectDrawStructs(view->selection().get());
	for(auto ods : odsList)
	{
		Point pos = ods->getPosition();
		Rect area = ods->getVisualArea();
		sum += pos.x + area.width() / 2;
	}

	float center = sum / odsList.size();

	auto& undoStack = view->getDocument()->getUndoStack();
	auto moveMacro = undoStack.beginMacro(view->tr("center on vertical axis").toStdString());

	for(auto ods : odsList)
	{
		Point pos = ods->getPosition();
		Rect area = ods->getVisualArea();
		Point delta = Point(center - pos.x - area.width() / 2, 0);
		if(!delta.isNull())
			undoStack.push(std::make_shared<MoveObjectCommand>(view, ods->getObject(), delta));
	}
}

void alignHorizontallyLeft(GraphView* view)
{
	float left = std::numeric_limits<float>::max();
	auto odsList = view->getObjectDrawStructs(view->selection().get());
	for(auto ods : odsList)
	{
		Point pos = ods->getPosition();
		left = std::min(pos.x, left);
	}

	auto& undoStack = view->getDocument()->getUndoStack();
	auto moveMacro = undoStack.beginMacro(view->tr("align left edges").toStdString());

	for(auto ods : odsList)
	{
		Point delta = Point(left - ods->getPosition().x, 0);
		if(!delta.isNull())
			undoStack.push(std::make_shared<MoveObjectCommand>(view, ods->getObject(), delta));
	}
}

void alignHorizontallyRight(GraphView* view)
{
	float right = std::numeric_limits<float>::lowest();
	auto odsList = view->getObjectDrawStructs(view->selection().get());
	for(auto ods : odsList)
	{
		Point pos = ods->getPosition();
		Rect area = ods->getVisualArea();
		right = std::max(pos.x + area.width(), right);
	}

	auto& undoStack = view->getDocument()->getUndoStack();
	auto moveMacro = undoStack.beginMacro(view->tr("align right edges").toStdString());

	for(auto ods : odsList)
	{
		Point pos = ods->getPosition();
		Rect area = ods->getVisualArea();
		Point delta = Point(right - pos.x - area.width(), 0);
		if(!delta.isNull())
			undoStack.push(std::make_shared<MoveObjectCommand>(view, ods->getObject(), delta));
	}
}

void alignVerticallyCenter(GraphView* view)
{
	float sum = 0;
	auto odsList = view->getObjectDrawStructs(view->selection().get());
	for(auto ods : odsList)
	{
		Point pos = ods->getPosition();
		Rect area = ods->getVisualArea();
		sum += pos.y + area.height() / 2;
	}

	float center = sum / odsList.size();

	auto& undoStack = view->getDocument()->getUndoStack();
	auto moveMacro = undoStack.beginMacro(view->tr("center on horizontal axis").toStdString());

	for(auto ods : odsList)
	{
		Point pos = ods->getPosition();
		Rect area = ods->getVisualArea();
		Point delta = Point(0, center - pos.y - area.height() / 2);
		if(!delta.isNull())
			undoStack.push(std::make_shared<MoveObjectCommand>(view, ods->getObject(), delta));
	}
}

void alignVerticallyTop(GraphView* view)
{
	float top = std::numeric_limits<float>::max();
	auto odsList = view->getObjectDrawStructs(view->selection().get());
	for(auto ods : odsList)
	{
		Point pos = ods->getPosition();
		top = std::min(pos.y, top);
	}

	auto& undoStack = view->getDocument()->getUndoStack();
	auto moveMacro = undoStack.beginMacro(view->tr("align top edges").toStdString());

	for(auto ods : odsList)
	{
		Point delta = Point(0, top - ods->getPosition().y);
		if(!delta.isNull())
			undoStack.push(std::make_shared<MoveObjectCommand>(view, ods->getObject(), delta));
	}
}

void alignVerticallyBottom(GraphView* view)
{
	float bottom = std::numeric_limits<float>::lowest();
	auto odsList = view->getObjectDrawStructs(view->selection().get());
	for(auto ods : odsList)
	{
		Point pos = ods->getPosition();
		Rect area = ods->getVisualArea();
		bottom = std::max(pos.y + area.height(), bottom);
	}

	auto& undoStack = view->getDocument()->getUndoStack();
	auto moveMacro = undoStack.beginMacro(view->tr("align bottom edges").toStdString());

	for(auto ods : odsList)
	{
		Point pos = ods->getPosition();
		Rect area = ods->getVisualArea();
		Point delta = Point(0, bottom - pos.y - area.height());
		if(!delta.isNull())
			undoStack.push(std::make_shared<MoveObjectCommand>(view, ods->getObject(), delta));
	}
}

void distributeHorizontallyCenter(GraphView* view)
{
	auto odsList = view->getObjectDrawStructs(view->selection().get());
	int nb = odsList.size();
	if (nb <= 2)
		return;

	using PosOds = std::pair<float, ObjectDrawStruct*>;
	std::vector<PosOds> positions;
	for(auto ods : odsList)
	{
		Point pos = ods->getPosition();
		Rect area = ods->getVisualArea();
		positions.emplace_back(pos.x + area.width() / 2, ods);
	}

	std::sort(positions.begin(), positions.end(), [](const PosOds& lhs, const PosOds& rhs) { return lhs.first < rhs.first; });

	float left = positions.front().first;
	float right = positions.back().first;
	float step = (right - left) / (nb - 1);

	auto& undoStack = view->getDocument()->getUndoStack();
	auto moveMacro = undoStack.beginMacro(view->tr("distribute centers horizontally").toStdString());

	for (int i = 1; i < nb; ++i)
	{
		const auto& posPair = positions[i];
		left += step;
		auto ods = posPair.second;
		Point pos = ods->getPosition();
		Rect area = ods->getVisualArea();
		Point delta = Point(left - pos.x - area.width() / 2, 0);
		if(!delta.isNull())
			undoStack.push(std::make_shared<MoveObjectCommand>(view, ods->getObject(), delta));
	}
}

void distributeHorizontallyGaps(GraphView* view)
{
	auto odsList = view->getObjectDrawStructs(view->selection().get());
	int nb = odsList.size();
	if (nb <= 2)
		return;

	using PosOds = std::pair<float, ObjectDrawStruct*>;
	std::vector<PosOds> positions;
	for(auto ods : odsList)
	{
		Point pos = ods->getPosition();
		Rect area = ods->getVisualArea();
		positions.emplace_back(pos.x, ods);
	}

	std::sort(positions.begin(), positions.end(), [](const PosOds& lhs, const PosOds& rhs) { return lhs.first < rhs.first; });

	float left = positions.front().first + positions.front().second->getVisualArea().width();
	float right = positions.back().first;
	
	// Compute total gap
	float width = 0;
	for (int i = 1; i < nb - 1; ++i)
		width += positions[i].second->getVisualArea().width();

	float step = (right - left - width) / (nb - 1);

	auto& undoStack = view->getDocument()->getUndoStack();
	auto moveMacro = undoStack.beginMacro(view->tr("distribute gaps horizontally").toStdString());

	for (int i = 1; i < nb; ++i)
	{
		const auto& posPair = positions[i];
		left += step;
		auto ods = posPair.second;
		Point pos = ods->getPosition();
		Rect area = ods->getVisualArea();
		Point delta = Point(left - pos.x, 0);
		if(!delta.isNull())
			undoStack.push(std::make_shared<MoveObjectCommand>(view, ods->getObject(), delta));

		left += area.width();
	}
}

void distributeHorizontallyLeft(GraphView* view)
{
	auto odsList = view->getObjectDrawStructs(view->selection().get());
	int nb = odsList.size();
	if (nb <= 2)
		return;

	using PosOds = std::pair<float, ObjectDrawStruct*>;
	std::vector<PosOds> positions;
	for(auto ods : odsList)
	{
		Point pos = ods->getPosition();
		Rect area = ods->getVisualArea();
		positions.emplace_back(pos.x, ods);
	}

	std::sort(positions.begin(), positions.end(), [](const PosOds& lhs, const PosOds& rhs) { return lhs.first < rhs.first; });

	float left = positions.front().first;
	float right = positions.back().first;
	float step = (right - left) / (nb - 1);

	auto& undoStack = view->getDocument()->getUndoStack();
	auto moveMacro = undoStack.beginMacro(view->tr("distribute left edges").toStdString());

	for (int i = 1; i < nb; ++i)
	{
		const auto& posPair = positions[i];
		left += step;
		auto ods = posPair.second;
		Point pos = ods->getPosition();
		Rect area = ods->getVisualArea();
		Point delta = Point(left - pos.x, 0);
		if(!delta.isNull())
			undoStack.push(std::make_shared<MoveObjectCommand>(view, ods->getObject(), delta));
	}
}

void distributeHorizontallyRight(GraphView* view)
{
	auto odsList = view->getObjectDrawStructs(view->selection().get());
	int nb = odsList.size();
	if (nb <= 2)
		return;

	using PosOds = std::pair<float, ObjectDrawStruct*>;
	std::vector<PosOds> positions;
	for(auto ods : odsList)
	{
		Point pos = ods->getPosition();
		Rect area = ods->getVisualArea();
		positions.emplace_back(pos.x + area.width(), ods);
	}

	std::sort(positions.begin(), positions.end(), [](const PosOds& lhs, const PosOds& rhs) { return lhs.first < rhs.first; });

	float left = positions.front().first;
	float right = positions.back().first;
	float step = (right - left) / (nb - 1);

	auto& undoStack = view->getDocument()->getUndoStack();
	auto moveMacro = undoStack.beginMacro(view->tr("distribute right edges").toStdString());

	for (int i = 1; i < nb; ++i)
	{
		const auto& posPair = positions[i];
		left += step;
		auto ods = posPair.second;
		Point pos = ods->getPosition();
		Rect area = ods->getVisualArea();
		Point delta = Point(left - pos.x - area.width(), 0);
		if(!delta.isNull())
			undoStack.push(std::make_shared<MoveObjectCommand>(view, ods->getObject(), delta));
	}
}

void distributeVerticallyCenter(GraphView* view)
{
	auto odsList = view->getObjectDrawStructs(view->selection().get());
	int nb = odsList.size();
	if (nb <= 2)
		return;

	using PosOds = std::pair<float, ObjectDrawStruct*>;
	std::vector<PosOds> positions;
	for(auto ods : odsList)
	{
		Point pos = ods->getPosition();
		Rect area = ods->getVisualArea();
		positions.emplace_back(pos.y + area.height() / 2, ods);
	}

	std::sort(positions.begin(), positions.end(), [](const PosOds& lhs, const PosOds& rhs) { return lhs.first < rhs.first; });

	float top = positions.front().first;
	float bottom = positions.back().first;
	float step = (bottom - top) / (nb - 1);

	auto& undoStack = view->getDocument()->getUndoStack();
	auto moveMacro = undoStack.beginMacro(view->tr("distribute centers vertically").toStdString());

	for (int i = 1; i < nb; ++i)
	{
		const auto& posPair = positions[i];
		top += step;
		auto ods = posPair.second;
		Point pos = ods->getPosition();
		Rect area = ods->getVisualArea();
		Point delta = Point(0, top - pos.y - area.height() / 2);
		if(!delta.isNull())
			undoStack.push(std::make_shared<MoveObjectCommand>(view, ods->getObject(), delta));
	}
}

void distributeVerticallyGaps(GraphView* view)
{
	auto odsList = view->getObjectDrawStructs(view->selection().get());
	int nb = odsList.size();
	if (nb <= 2)
		return;

	using PosOds = std::pair<float, ObjectDrawStruct*>;
	std::vector<PosOds> positions;
	for(auto ods : odsList)
	{
		Point pos = ods->getPosition();
		Rect area = ods->getVisualArea();
		positions.emplace_back(pos.y, ods);
	}

	std::sort(positions.begin(), positions.end(), [](const PosOds& lhs, const PosOds& rhs) { return lhs.first < rhs.first; });

	float top = positions.front().first + positions.front().second->getVisualArea().height();
	float bottom = positions.back().first;
	
	// Compute total gap
	float height = 0;
	for (int i = 1; i < nb - 1; ++i)
		height += positions[i].second->getVisualArea().height();

	float step = (bottom - top - height) / (nb - 1);

	auto& undoStack = view->getDocument()->getUndoStack();
	auto moveMacro = undoStack.beginMacro(view->tr("distribute gaps vertically").toStdString());

	for (int i = 1; i < nb; ++i)
	{
		const auto& posPair = positions[i];
		top += step;
		auto ods = posPair.second;
		Point pos = ods->getPosition();
		Rect area = ods->getVisualArea();
		Point delta = Point(0, top - pos.y);
		if(!delta.isNull())
			undoStack.push(std::make_shared<MoveObjectCommand>(view, ods->getObject(), delta));

		top += area.height();
	}
}

void distributeVerticallyTop(GraphView* view)
{
	auto odsList = view->getObjectDrawStructs(view->selection().get());
	int nb = odsList.size();
	if (nb <= 2)
		return;

	using PosOds = std::pair<float, ObjectDrawStruct*>;
	std::vector<PosOds> positions;
	for(auto ods : odsList)
	{
		Point pos = ods->getPosition();
		Rect area = ods->getVisualArea();
		positions.emplace_back(pos.y, ods);
	}

	std::sort(positions.begin(), positions.end(), [](const PosOds& lhs, const PosOds& rhs) { return lhs.first < rhs.first; });

	float top = positions.front().first;
	float bottom = positions.back().first;
	float step = (bottom - top) / (nb - 1);

	auto& undoStack = view->getDocument()->getUndoStack();
	auto moveMacro = undoStack.beginMacro(view->tr("distribute top edges").toStdString());

	for (int i = 1; i < nb; ++i)
	{
		const auto& posPair = positions[i];
		top += step;
		auto ods = posPair.second;
		Point pos = ods->getPosition();
		Rect area = ods->getVisualArea();
		Point delta = Point(0, top - pos.y);
		if(!delta.isNull())
			undoStack.push(std::make_shared<MoveObjectCommand>(view, ods->getObject(), delta));
	}
}

void distributeVerticallyBottom(GraphView* view)
{
	auto odsList = view->getObjectDrawStructs(view->selection().get());
	int nb = odsList.size();
	if (nb <= 2)
		return;

	using PosOds = std::pair<float, ObjectDrawStruct*>;
	std::vector<PosOds> positions;
	for(auto ods : odsList)
	{
		Point pos = ods->getPosition();
		Rect area = ods->getVisualArea();
		positions.emplace_back(pos.y + area.height(), ods);
	}

	std::sort(positions.begin(), positions.end(), [](const PosOds& lhs, const PosOds& rhs) { return lhs.first < rhs.first; });

	float top = positions.front().first;
	float bottom = positions.back().first;
	float step = (bottom - top) / (nb - 1);

	auto& undoStack = view->getDocument()->getUndoStack();
	auto moveMacro = undoStack.beginMacro(view->tr("distribute bottom edges").toStdString());

	for (int i = 1; i < nb; ++i)
	{
		const auto& posPair = positions[i];
		top += step;
		auto ods = posPair.second;
		Point pos = ods->getPosition();
		Rect area = ods->getVisualArea();
		Point delta = Point(0, top - pos.y - area.height());
		if(!delta.isNull())
			undoStack.push(std::make_shared<MoveObjectCommand>(view, ods->getObject(), delta));
	}
}
