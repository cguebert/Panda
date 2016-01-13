#include <ui/graph/alignObjects.h>
#include <ui/GraphView.h>
#include <ui/drawstruct/ObjectDrawStruct.h>
#include <ui/command/MoveObjectCommand.h>

#include <panda/PandaDocument.h>

void alignHorizontallyCenter(GraphView* view)
{
	qreal sum = 0;
	auto document = view->getDocument();
	auto odsList = view->getObjectDrawStructs(document->getSelection());
	for(auto ods : odsList)
	{
		QPointF pos = ods->getPosition();
		QRectF area = ods->getObjectArea();
		sum += pos.x() + area.width() / 2;
	}

	qreal center = sum / odsList.size();

	auto moveMacro = document->beginCommandMacro(view->tr("center on vertical axis").toStdString());

	for(auto ods : odsList)
	{
		QPointF pos = ods->getPosition();
		QRectF area = ods->getObjectArea();
		QPointF delta = QPointF(center - pos.x() - area.width() / 2, 0);
		if(!delta.isNull())
			document->addCommand(std::make_shared<MoveObjectCommand>(view, ods->getObject(), delta));
	}
}

void alignHorizontallyLeft(GraphView* view)
{
	qreal left = std::numeric_limits<qreal>::max();
	auto document = view->getDocument();
	auto odsList = view->getObjectDrawStructs(document->getSelection());
	for(auto ods : odsList)
	{
		QPointF pos = ods->getPosition();
		left = std::min(pos.x(), left);
	}

	auto moveMacro = document->beginCommandMacro(view->tr("align left edges").toStdString());

	for(auto ods : odsList)
	{
		QPointF delta = QPointF(left - ods->getPosition().x(), 0);
		if(!delta.isNull())
			document->addCommand(std::make_shared<MoveObjectCommand>(view, ods->getObject(), delta));
	}
}

void alignHorizontallyRight(GraphView* view)
{
	qreal right = std::numeric_limits<qreal>::lowest();
	auto document = view->getDocument();
	auto odsList = view->getObjectDrawStructs(document->getSelection());
	for(auto ods : odsList)
	{
		QPointF pos = ods->getPosition();
		QRectF area = ods->getObjectArea();
		right = std::max(pos.x() + area.width(), right);
	}

	auto moveMacro = document->beginCommandMacro(view->tr("align right edges").toStdString());

	for(auto ods : odsList)
	{
		QPointF pos = ods->getPosition();
		QRectF area = ods->getObjectArea();
		QPointF delta = QPointF(right - pos.x() - area.width(), 0);
		if(!delta.isNull())
			document->addCommand(std::make_shared<MoveObjectCommand>(view, ods->getObject(), delta));
	}
}

void alignVerticallyCenter(GraphView* view)
{
	qreal sum = 0;
	auto document = view->getDocument();
	auto odsList = view->getObjectDrawStructs(document->getSelection());
	for(auto ods : odsList)
	{
		QPointF pos = ods->getPosition();
		QRectF area = ods->getObjectArea();
		sum += pos.y() + area.height() / 2;
	}

	qreal center = sum / odsList.size();

	auto moveMacro = document->beginCommandMacro(view->tr("center on horizontal axis").toStdString());

	for(auto ods : odsList)
	{
		QPointF pos = ods->getPosition();
		QRectF area = ods->getObjectArea();
		QPointF delta = QPointF(0, center - pos.y() - area.height() / 2);
		if(!delta.isNull())
			document->addCommand(std::make_shared<MoveObjectCommand>(view, ods->getObject(), delta));
	}
}

void alignVerticallyTop(GraphView* view)
{
	qreal top = std::numeric_limits<qreal>::max();
	auto document = view->getDocument();
	auto odsList = view->getObjectDrawStructs(document->getSelection());
	for(auto ods : odsList)
	{
		QPointF pos = ods->getPosition();
		top = std::min(pos.y(), top);
	}

	auto moveMacro = document->beginCommandMacro(view->tr("align top edges").toStdString());

	for(auto ods : odsList)
	{
		QPointF delta = QPointF(0, top - ods->getPosition().y());
		if(!delta.isNull())
			document->addCommand(std::make_shared<MoveObjectCommand>(view, ods->getObject(), delta));
	}
}

void alignVerticallyBottom(GraphView* view)
{
	qreal bottom = std::numeric_limits<qreal>::lowest();
	auto document = view->getDocument();
	auto odsList = view->getObjectDrawStructs(document->getSelection());
	for(auto ods : odsList)
	{
		QPointF pos = ods->getPosition();
		QRectF area = ods->getObjectArea();
		bottom = std::max(pos.y() + area.height(), bottom);
	}

	auto moveMacro = document->beginCommandMacro(view->tr("align bottom edges").toStdString());

	for(auto ods : odsList)
	{
		QPointF pos = ods->getPosition();
		QRectF area = ods->getObjectArea();
		QPointF delta = QPointF(0, bottom - pos.y() - area.height());
		if(!delta.isNull())
			document->addCommand(std::make_shared<MoveObjectCommand>(view, ods->getObject(), delta));
	}
}

void distributeHorizontallyCenter(GraphView* view)
{
	auto document = view->getDocument();
	auto odsList = view->getObjectDrawStructs(document->getSelection());
	int nb = odsList.size();
	if (nb <= 2)
		return;

	using PosOds = std::pair<qreal, ObjectDrawStruct*>;
	std::vector<PosOds> positions;
	for(auto ods : odsList)
	{
		QPointF pos = ods->getPosition();
		QRectF area = ods->getObjectArea();
		positions.emplace_back(pos.x() + area.width() / 2, ods);
	}

	std::sort(positions.begin(), positions.end(), [](const PosOds& lhs, const PosOds& rhs) { return lhs.first < rhs.first; });

	qreal left = positions.front().first;
	qreal right = positions.back().first;
	qreal step = (right - left) / (nb - 1);

	auto moveMacro = document->beginCommandMacro(view->tr("distribute centers horizontally").toStdString());

	for (int i = 1; i < nb; ++i)
	{
		const auto& posPair = positions[i];
		left += step;
		auto ods = posPair.second;
		QPointF pos = ods->getPosition();
		QRectF area = ods->getObjectArea();
		QPointF delta = QPointF(left - pos.x() - area.width() / 2, 0);
		if(!delta.isNull())
			document->addCommand(std::make_shared<MoveObjectCommand>(view, ods->getObject(), delta));
	}
}

void distributeHorizontallyGaps(GraphView* view)
{
	auto document = view->getDocument();
	auto odsList = view->getObjectDrawStructs(document->getSelection());
	int nb = odsList.size();
	if (nb <= 2)
		return;

	using PosOds = std::pair<qreal, ObjectDrawStruct*>;
	std::vector<PosOds> positions;
	for(auto ods : odsList)
	{
		QPointF pos = ods->getPosition();
		QRectF area = ods->getObjectArea();
		positions.emplace_back(pos.x(), ods);
	}

	std::sort(positions.begin(), positions.end(), [](const PosOds& lhs, const PosOds& rhs) { return lhs.first < rhs.first; });

	qreal left = positions.front().first + positions.front().second->getObjectArea().width();
	qreal right = positions.back().first;
	
	// Compute total gap
	qreal width = 0;
	for (int i = 1; i < nb - 1; ++i)
		width += positions[i].second->getObjectArea().width();

	qreal step = (right - left - width) / (nb - 1);

	auto moveMacro = document->beginCommandMacro(view->tr("distribute gaps horizontally").toStdString());

	for (int i = 1; i < nb; ++i)
	{
		const auto& posPair = positions[i];
		left += step;
		auto ods = posPair.second;
		QPointF pos = ods->getPosition();
		QRectF area = ods->getObjectArea();
		QPointF delta = QPointF(left - pos.x(), 0);
		if(!delta.isNull())
			document->addCommand(std::make_shared<MoveObjectCommand>(view, ods->getObject(), delta));

		left += area.width();
	}
}

void distributeHorizontallyLeft(GraphView* view)
{
	auto document = view->getDocument();
	auto odsList = view->getObjectDrawStructs(document->getSelection());
	int nb = odsList.size();
	if (nb <= 2)
		return;

	using PosOds = std::pair<qreal, ObjectDrawStruct*>;
	std::vector<PosOds> positions;
	for(auto ods : odsList)
	{
		QPointF pos = ods->getPosition();
		QRectF area = ods->getObjectArea();
		positions.emplace_back(pos.x(), ods);
	}

	std::sort(positions.begin(), positions.end(), [](const PosOds& lhs, const PosOds& rhs) { return lhs.first < rhs.first; });

	qreal left = positions.front().first;
	qreal right = positions.back().first;
	qreal step = (right - left) / (nb - 1);

	auto moveMacro = document->beginCommandMacro(view->tr("distribute left edges").toStdString());

	for (int i = 1; i < nb; ++i)
	{
		const auto& posPair = positions[i];
		left += step;
		auto ods = posPair.second;
		QPointF pos = ods->getPosition();
		QRectF area = ods->getObjectArea();
		QPointF delta = QPointF(left - pos.x(), 0);
		if(!delta.isNull())
			document->addCommand(std::make_shared<MoveObjectCommand>(view, ods->getObject(), delta));
	}
}

void distributeHorizontallyRight(GraphView* view)
{
	auto document = view->getDocument();
	auto odsList = view->getObjectDrawStructs(document->getSelection());
	int nb = odsList.size();
	if (nb <= 2)
		return;

	using PosOds = std::pair<qreal, ObjectDrawStruct*>;
	std::vector<PosOds> positions;
	for(auto ods : odsList)
	{
		QPointF pos = ods->getPosition();
		QRectF area = ods->getObjectArea();
		positions.emplace_back(pos.x() + area.width(), ods);
	}

	std::sort(positions.begin(), positions.end(), [](const PosOds& lhs, const PosOds& rhs) { return lhs.first < rhs.first; });

	qreal left = positions.front().first;
	qreal right = positions.back().first;
	qreal step = (right - left) / (nb - 1);

	auto moveMacro = document->beginCommandMacro(view->tr("distribute right edges").toStdString());

	for (int i = 1; i < nb; ++i)
	{
		const auto& posPair = positions[i];
		left += step;
		auto ods = posPair.second;
		QPointF pos = ods->getPosition();
		QRectF area = ods->getObjectArea();
		QPointF delta = QPointF(left - pos.x() - area.width(), 0);
		if(!delta.isNull())
			document->addCommand(std::make_shared<MoveObjectCommand>(view, ods->getObject(), delta));
	}
}

void distributeVerticallyCenter(GraphView* view)
{
	auto document = view->getDocument();
	auto odsList = view->getObjectDrawStructs(document->getSelection());
	int nb = odsList.size();
	if (nb <= 2)
		return;

	using PosOds = std::pair<qreal, ObjectDrawStruct*>;
	std::vector<PosOds> positions;
	for(auto ods : odsList)
	{
		QPointF pos = ods->getPosition();
		QRectF area = ods->getObjectArea();
		positions.emplace_back(pos.y() + area.height() / 2, ods);
	}

	std::sort(positions.begin(), positions.end(), [](const PosOds& lhs, const PosOds& rhs) { return lhs.first < rhs.first; });

	qreal top = positions.front().first;
	qreal bottom = positions.back().first;
	qreal step = (bottom - top) / (nb - 1);

	auto moveMacro = document->beginCommandMacro(view->tr("distribute centers vertically").toStdString());

	for (int i = 1; i < nb; ++i)
	{
		const auto& posPair = positions[i];
		top += step;
		auto ods = posPair.second;
		QPointF pos = ods->getPosition();
		QRectF area = ods->getObjectArea();
		QPointF delta = QPointF(0, top - pos.y() - area.height() / 2);
		if(!delta.isNull())
			document->addCommand(std::make_shared<MoveObjectCommand>(view, ods->getObject(), delta));
	}
}

void distributeVerticallyGaps(GraphView* view)
{
	auto document = view->getDocument();
	auto odsList = view->getObjectDrawStructs(document->getSelection());
	int nb = odsList.size();
	if (nb <= 2)
		return;

	using PosOds = std::pair<qreal, ObjectDrawStruct*>;
	std::vector<PosOds> positions;
	for(auto ods : odsList)
	{
		QPointF pos = ods->getPosition();
		QRectF area = ods->getObjectArea();
		positions.emplace_back(pos.y(), ods);
	}

	std::sort(positions.begin(), positions.end(), [](const PosOds& lhs, const PosOds& rhs) { return lhs.first < rhs.first; });

	qreal top = positions.front().first + positions.front().second->getObjectArea().height();
	qreal bottom = positions.back().first;
	
	// Compute total gap
	qreal height = 0;
	for (int i = 1; i < nb - 1; ++i)
		height += positions[i].second->getObjectArea().height();

	qreal step = (bottom - top - height) / (nb - 1);

	auto moveMacro = document->beginCommandMacro(view->tr("distribute gaps vertically").toStdString());

	for (int i = 1; i < nb; ++i)
	{
		const auto& posPair = positions[i];
		top += step;
		auto ods = posPair.second;
		QPointF pos = ods->getPosition();
		QRectF area = ods->getObjectArea();
		QPointF delta = QPointF(0, top - pos.y());
		if(!delta.isNull())
			document->addCommand(std::make_shared<MoveObjectCommand>(view, ods->getObject(), delta));

		top += area.height();
	}
}

void distributeVerticallyTop(GraphView* view)
{
	auto document = view->getDocument();
	auto odsList = view->getObjectDrawStructs(document->getSelection());
	int nb = odsList.size();
	if (nb <= 2)
		return;

	using PosOds = std::pair<qreal, ObjectDrawStruct*>;
	std::vector<PosOds> positions;
	for(auto ods : odsList)
	{
		QPointF pos = ods->getPosition();
		QRectF area = ods->getObjectArea();
		positions.emplace_back(pos.y(), ods);
	}

	std::sort(positions.begin(), positions.end(), [](const PosOds& lhs, const PosOds& rhs) { return lhs.first < rhs.first; });

	qreal top = positions.front().first;
	qreal bottom = positions.back().first;
	qreal step = (bottom - top) / (nb - 1);

	auto moveMacro = document->beginCommandMacro(view->tr("distribute top edges").toStdString());

	for (int i = 1; i < nb; ++i)
	{
		const auto& posPair = positions[i];
		top += step;
		auto ods = posPair.second;
		QPointF pos = ods->getPosition();
		QRectF area = ods->getObjectArea();
		QPointF delta = QPointF(0, top - pos.y());
		if(!delta.isNull())
			document->addCommand(std::make_shared<MoveObjectCommand>(view, ods->getObject(), delta));
	}
}

void distributeVerticallyBottom(GraphView* view)
{
	auto document = view->getDocument();
	auto odsList = view->getObjectDrawStructs(document->getSelection());
	int nb = odsList.size();
	if (nb <= 2)
		return;

	using PosOds = std::pair<qreal, ObjectDrawStruct*>;
	std::vector<PosOds> positions;
	for(auto ods : odsList)
	{
		QPointF pos = ods->getPosition();
		QRectF area = ods->getObjectArea();
		positions.emplace_back(pos.y() + area.height(), ods);
	}

	std::sort(positions.begin(), positions.end(), [](const PosOds& lhs, const PosOds& rhs) { return lhs.first < rhs.first; });

	qreal top = positions.front().first;
	qreal bottom = positions.back().first;
	qreal step = (bottom - top) / (nb - 1);

	auto moveMacro = document->beginCommandMacro(view->tr("distribute bottom edges").toStdString());

	for (int i = 1; i < nb; ++i)
	{
		const auto& posPair = positions[i];
		top += step;
		auto ods = posPair.second;
		QPointF pos = ods->getPosition();
		QRectF area = ods->getObjectArea();
		QPointF delta = QPointF(0, top - pos.y() - area.height());
		if(!delta.isNull())
			document->addCommand(std::make_shared<MoveObjectCommand>(view, ods->getObject(), delta));
	}
}
