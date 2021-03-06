#include <ui/graphview/QtViewWrapper.h>

#include <QtWidgets>
#include <functional>
#include <limits>

#include <ui/MainWindow.h>
#include <ui/dialog/ChooseWidgetDialog.h>
#include <ui/dialog/QuickCreateDialog.h>
#include <ui/graphview/QtViewGUI.h>
#include <ui/graphview/QtViewRenderer.h>

#include <panda/graphview/object/ObjectRenderer.h>
#include <panda/graphview/object/DockableRenderer.h>
#include <panda/graphview/DataLabelAddon.h>
#include <panda/graphview/GraphView.h>
#include <panda/graphview/LinksList.h>
#include <panda/graphview/LinkTagsList.h>
#include <panda/graphview/ObjectsSelection.h>
#include <panda/graphview/ObjectRenderersList.h>
#include <panda/graphview/ViewInteraction.h>
#include <panda/graphview/Viewport.h>
#include <panda/graphview/graphics/DrawList.h>

#include <panda/GroupsManager.h>
#include <panda/SimpleGUI.h>
#include <panda/VisualizersManager.h>
#include <panda/command/AddObjectCommand.h>
#include <panda/command/DockableCommand.h>
#include <panda/command/LinkDatasCommand.h>
#include <panda/command/RemoveObjectCommand.h>
#include <panda/document/DocumentSignals.h>
#include <panda/document/ObjectsList.h>
#include <panda/document/PandaDocument.h>
#include <panda/document/Serialization.h>
#include <panda/helper/algorithm.h>
#include <panda/object/Group.h>
#include <panda/object/ObjectFactory.h>
#include <panda/object/visualizer/CustomVisualizer.h>
#include <panda/object/visualizer/VisualizerDocument.h>
#include <panda/types/DataTraits.h>

#ifdef PANDA_LOG_EVENTS
#include <ui/dialog/UpdateLoggerDialog.h>
#endif

using Point = panda::types::Point;
using Rect = panda::types::Rect;
using DrawList = panda::graphview::graphics::DrawList;
using DrawColors = panda::graphview::graphics::DrawColors;

namespace
{
	inline panda::types::Point convert(const QPointF& pt)
	{ return { static_cast<float>(pt.x()), static_cast<float>(pt.y()) }; }

	inline panda::types::Point convert(const QPoint& pt)
	{ return { static_cast<float>(pt.x()), static_cast<float>(pt.y()) }; }

	inline panda::types::Rect convert(const QRect& r)
	{ return panda::types::Rect(r.left(), r.top(), r.right(), r.bottom()); }

	inline QPoint convert(const panda::types::Point& pt)
	{ return QPointF{ pt.x, pt.y }.toPoint(); }

	panda::graphview::EventModifiers convert(Qt::KeyboardModifiers modifiers)
	{
		panda::graphview::EventModifiers mod = panda::graphview::EventModifier::NoModifier;
		if (modifiers & Qt::KeyboardModifier::ShiftModifier)   mod |= panda::graphview::EventModifier::ShiftModifier;
		if (modifiers & Qt::KeyboardModifier::ControlModifier) mod |= panda::graphview::EventModifier::ControlModifier;
		if (modifiers & Qt::KeyboardModifier::AltModifier)     mod |= panda::graphview::EventModifier::AltModifier;

		return mod;
	}

	panda::graphview::MouseButton convert(Qt::MouseButton button)
	{
		if (button == Qt::MouseButton::LeftButton)   return panda::graphview::MouseButton::LeftButton;
		if (button == Qt::MouseButton::RightButton)  return panda::graphview::MouseButton::RightButton;
		if (button == Qt::MouseButton::MiddleButton) return panda::graphview::MouseButton::MiddleButton;

		return panda::graphview::MouseButton::NoButton;
	}

	unsigned int convert(const QColor& col)
	{
		unsigned int out;
		out = col.red() & 0xFF;
		out |= (col.green() & 0xFF) << 8;
		out |= (col.blue() & 0xFF) << 16;
		out |= (col.alpha() & 0xFF) << 24;
		return out;
	}
}

namespace graphview
{

QtViewWrapper::QtViewWrapper(std::unique_ptr<panda::graphview::GraphView> graphView, MainWindow* mainWindow)
	: QOpenGLWidget(mainWindow)
	, m_graphView(std::move(graphView))
	, m_viewRenderer(std::make_shared<QtViewRenderer>())
{
	QSurfaceFormat fmt;
	fmt.setSamples(8);
	setFormat(fmt);

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	setFocusPolicy(Qt::StrongFocus);

	setMouseTracking(true);

	auto viewGui = std::make_unique<QtViewGui>(*this, mainWindow);

	const auto& pal = palette();
	m_drawColors.penColor = convert(pal.text().color());
	m_drawColors.midLightColor = convert(pal.midlight().color());
	m_drawColors.lightColor = convert(pal.light().color());
	m_drawColors.highlightColor = convert(pal.highlight().color());
	m_drawColors.backgroundColor = convert(pal.highlight().color());

	auto clearColor = palette().background().color();
	m_viewRenderer->setClearColor(clearColor.redF(), clearColor.greenF(), clearColor.blueF());
	m_graphView->setGui(m_viewRenderer, std::move(viewGui));

	m_observer.get(m_graphView->updateNeeded).connect<QWidget, &QWidget::update>(this);
	m_observer.get(m_graphView->modified).connect<QWidget, &QWidget::update>(this);
	m_observer.get(m_graphView->viewportModified).connect<QtViewWrapper, &QtViewWrapper::emitViewportModified>(this);
}

QSize QtViewWrapper::minimumSizeHint() const
{
	return QSize(300, 200);
}

QSize QtViewWrapper::sizeHint() const
{
	return QSize(600, 400);
}

void QtViewWrapper::initializeGL()
{
	m_viewRenderer->initialize();
	m_drawList = std::make_shared<DrawList>(*m_viewRenderer);
	m_graphView->initializeRenderer(*m_viewRenderer);
}

void QtViewWrapper::resizeGL(int w, int h)
{
	m_viewRenderer->resize(w, h);
	m_graphView->resizeView(w, h);
	update();
}

void QtViewWrapper::paintGL()
{
	decltype(m_functionsToExecuteNextRefresh) functions;
	functions.swap(m_functionsToExecuteNextRefresh);
	for (const auto func : functions)
		func();

	m_graphView->beforeDraw();

	// Prepare the renderer
	const auto displayRect = m_graphView->viewport().displayRect();
	m_viewRenderer->setView(displayRect);
	m_viewRenderer->newFrame();

	m_graphView->drawGraphView(*m_viewRenderer, m_drawColors);

	if (m_debugDirtyState)
	{
		m_drawList->clear();
#ifdef PANDA_LOG_EVENTS
		UpdateLoggerDialog* logDlg = UpdateLoggerDialog::getInstance();
		if(logDlg && logDlg->isVisible())
			paintLogDebug(*m_drawList, m_drawColors);
		else
#endif
			paintDirtyState(*m_drawList, m_drawColors);
		m_viewRenderer->addDrawList(m_drawList);
	}

	// Execute the render commands
	m_viewRenderer->render();
}

#ifdef PANDA_LOG_EVENTS
void QtViewWrapper::paintLogDebug(DrawList& list, DrawColors& colors)
{
	UpdateLoggerDialog* logDlg = UpdateLoggerDialog::getInstance();
	if(logDlg && logDlg->isVisible())
	{
		const auto& states = logDlg->getNodeStates();
		for(const auto objRnd : m_graphView->objectRenderers().getOrdered())
		{
			const auto object = objRnd->getObject();
			unsigned int fillCol = panda::helper::valueOrDefault(states, object, nullptr) ? 0x200000FF : 0x2000FF00;
	
			auto area = objRnd->getVisualArea();
			list.addRectFilled(area, fillCol);

			for(panda::BaseData* data : object->getDatas())
			{
				if(objRnd->getDataRect(data, area))
				{
					fillCol = panda::helper::valueOrDefault(states, data, nullptr) ? 0x400000FF : 0x4000FF00;
					list.addRectFilled(area, fillCol);
				}
			}
		}

		const panda::helper::EventData* event = logDlg->getSelectedEvent();
		if(event)
		{
			panda::PandaObject* object = m_graphView->objectsList().find(event->m_objectIndex);
			if(object)
			{
				auto objRnd = m_graphView->objectRenderers().get(object);
				Rect area;

				bool drawData = false;
				const panda::BaseData* data = dynamic_cast<const panda::BaseData*>(event->m_node);
				if(data)
					drawData = objRnd->getDataRect(data, area);
				if(!drawData)
					area = objRnd->getVisualArea();

				list.addRectFilled(area, 0x80FF8080);
			}
		}
	}
}
#endif

void QtViewWrapper::paintDirtyState(DrawList& list, DrawColors& colors)
{
	for(const auto& objRnd : m_graphView->objectRenderers().getOrdered())
	{
		const auto object = objRnd->getObject();
		unsigned int fillCol = object->isDirty() ? 0x400000FF : 0x4000FF00;

		auto area = objRnd->getVisualArea();
		list.addRectFilled(area, fillCol);

		for(panda::BaseData* data : object->getDatas())
		{
			Rect area;
			if(objRnd->getDataRect(data, area))
			{
				fillCol = data->isDirty() ? 0x400000FF : 0x4000FF00;
				list.addRectFilled(area, fillCol);
			}
		}
	}
}

void QtViewWrapper::mousePressEvent(QMouseEvent* event)
{
	m_graphView->interaction().mousePressEvent({ convert(event->pos()), convert(event->button()), convert(event->modifiers()) });
}

void QtViewWrapper::mouseMoveEvent(QMouseEvent* event)
{
	m_graphView->interaction().mouseMoveEvent({ convert(event->pos()), convert(event->button()), convert(event->modifiers()) });
}

void QtViewWrapper::mouseReleaseEvent(QMouseEvent* event)
{
	m_graphView->interaction().mouseReleaseEvent({ convert(event->pos()), convert(event->button()), convert(event->modifiers()) });
}

void QtViewWrapper::wheelEvent(QWheelEvent* event)
{
	m_graphView->interaction().wheelEvent({ convert(event->pos()), convert(event->angleDelta()), convert(event->modifiers()) });
}

void QtViewWrapper::keyPressEvent(QKeyEvent* event)
{
	panda::graphview::KeyEvent ke { static_cast<panda::graphview::Key>(event->key()), convert(event->modifiers()) };
	if (!m_graphView->interaction().keyPressEvent(ke))
	{
		if(event->key() == Qt::Key_Space && !m_graphView->document()->animationIsPlaying())
			QuickCreateDialog { m_graphView->document(), m_graphView.get(), this }.exec();
		else
			QWidget::keyPressEvent(event);
	}
}

void QtViewWrapper::contextMenuEvent(QContextMenuEvent* event)
{
	m_graphView->interaction().contextMenuEvent({ convert(event->pos()), convert(event->modifiers()) });
}

QSize QtViewWrapper::viewSize()
{
	const auto& viewport = m_graphView->viewport();
	const auto& viewRect = viewport.viewRect();
	return QSize(viewRect.width(), viewRect.height());
}

QPoint QtViewWrapper::viewPosition()
{
	const auto& viewport = m_graphView->viewport();
	const auto& viewRect = viewport.viewRect();
	auto delta = viewport.viewDelta() * viewport.zoom();
	return QPoint(viewRect.left() - delta.x, viewRect.top() - delta.y);
}

void QtViewWrapper::scrollView(QPoint position)
{
	auto& viewport = m_graphView->viewport();
	const auto& viewRect = viewport.viewRect();
	Point delta = convert(position) - viewRect.topLeft() + viewport.viewDelta() * viewport.zoom();
	viewport.moveView(delta);
}

void QtViewWrapper::showChooseWidgetDialog()
{
	auto contextMenuData = m_graphView->interaction().contextMenuData();
	if (contextMenuData)
		ChooseWidgetDialog(contextMenuData, this).exec();
	else
	{
		auto obj = m_graphView->selection().lastSelectedObject();
		if(obj && obj->getClass()->getClassName() == "GeneratorUser" && obj->getClass()->getNamespaceName() == "panda")
		{
			auto data = obj->getData("input");
			if(data)
				ChooseWidgetDialog(data, this).exec();
		}
	}
}

void QtViewWrapper::focusOutEvent(QFocusEvent*)
{
	emit lostFocus(this);
}

void QtViewWrapper::setDataLabel()
{
	auto data = m_graphView->interaction().contextMenuData();
	if (!data)
		return;

	auto label = panda::graphview::DataLabelAddon::getDataLabel(data);

	bool ok = false;
	label = QInputDialog::getMultiLineText(this, tr("Data label"), tr("Label:"), QString::fromStdString(label), &ok).toStdString();
	if (!ok)
		return;

	panda::graphview::DataLabelAddon::setDataLabel(data, label);

	emit modified();
}

void QtViewWrapper::createVisualizer()
{
	auto data = m_graphView->interaction().contextMenuData();
	if (!data)
		return;

	const auto visualizers = panda::VisualizersManager::visualizers(data->getDataTrait()->fullTypeId());

	if (visualizers.empty())
		return;

	std::string path;
	if (visualizers.size() == 1)
		path = visualizers.front().path;
	else
	{
		QStringList items;
		for (const auto& v : visualizers)
			items << QString::fromStdString(v.name);
		bool ok = false;
		auto item = QInputDialog::getItem(this, "Create data visualizer", "Choose the type", items, 0, false, &ok);
		if (!ok)
			return;

		const auto index = items.indexOf(item);
		path = visualizers[index].path;
	}

	try 
	{
		auto document = m_graphView->document();
		auto object = panda::ObjectFactory::create("panda::CustomVisualizer", document);

		auto& undo = document->getUndoStack();
		auto macro = undo.beginMacro("create visualizer");
		
		undo.push(std::make_shared<panda::AddObjectCommand>(document, document->getObjectsList(), object));

		auto visualizer = std::dynamic_pointer_cast<panda::CustomVisualizer>(object);
		if (visualizer)
		{
			visualizer->setDocumentPath(path);
			auto visuData = visualizer->visualizedData();
			if(visuData)
				undo.push(std::make_shared<panda::LinkDatasCommand>(visuData, data));
		}

		emit modified();
	}
	catch (const std::exception& e)
	{
		QMessageBox::warning(this, "Load error", QString::fromLocal8Bit(e.what()));
	}
}

void QtViewWrapper::copy()
{
	const auto& selected = m_graphView->selection().get();
	if (selected.empty())
		return;

	try
	{
		QApplication::clipboard()->setText(QString::fromStdString(panda::serialization::writeTextDocument(m_graphView->document(), selected)));
	}
	catch (const std::exception&)
	{
	}
}

void QtViewWrapper::cut()
{
	copy();
	del();
}

void QtViewWrapper::paste()
{
	const QMimeData* mimeData = QApplication::clipboard()->mimeData();
	if (!mimeData->hasText())
		return;
	
	panda::serialization::Objects result;
	try
	{
		result = panda::serialization::readTextDocument(m_graphView->document(), m_graphView->objectsList(), mimeData->text().toStdString());
	}
	catch (const std::exception& e)
	{
		QMessageBox::warning(this, "Import error", QString::fromLocal8Bit(e.what()));
		return;
	}

	m_graphView->selection().set(result);
	m_graphView->viewport().moveSelectedToCenter();

	m_graphView->document()->getUndoStack().push(std::make_shared<panda::AddObjectCommand>(m_graphView->document(), m_graphView->objectsList(), result));
}

void QtViewWrapper::del()
{
	const auto& selected = m_graphView->selection().get();
	if (selected.empty())
		return;

	auto macro = m_graphView->document()->getUndoStack().beginMacro(tr("delete objects").toStdString());	
	m_graphView->document()->getUndoStack().push(std::make_shared<panda::RemoveObjectCommand>(m_graphView->document(), m_graphView->objectsList(), selected));
}

void QtViewWrapper::executeNextRefresh(std::function<void()> func)
{
	m_functionsToExecuteNextRefresh.push_back(func);
}

void QtViewWrapper::emitViewportModified()
{
	emit viewportModified();
}

void QtViewWrapper::saveGroup()
{
	panda::PandaObject* object = m_graphView->selection().lastSelectedObject();
	panda::Group* group = dynamic_cast<panda::Group*>(object);
	if (!group)
	{
		QMessageBox::warning(this, "Save group error", "The selected objet is not a group");
		return;
	}

	bool ok;
	const auto text = QInputDialog::getText(nullptr, tr("Save group"),
											tr("Group name:"), QLineEdit::Normal,
											QString::fromStdString(group->getGroupName()), &ok);
	if (!ok || text.isEmpty())
		return;

	const auto groupPath = text.toStdString();
	QString description;
	auto info = panda::GroupsManager::groupInformation(groupPath);

	// It already exists
	if (info)
	{
		if (QMessageBox::question(nullptr, tr("Panda"),
									tr("This group already exists, overwrite?"),
									QMessageBox::Yes | QMessageBox::No,
									QMessageBox::Yes) != QMessageBox::Yes)
			return;
		description = QString::fromStdString(info->description);
	}

	description = QInputDialog::getText(nullptr, tr("Save group"),
										tr("Group description:"), QLineEdit::Normal,
										description, &ok);

	if(panda::GroupsManager::saveGroup(group, groupPath, description.toStdString()))
	{
		m_graphView->gui().setStatusBarMessage("Group saved");
		emit groupsListModified();
	}
}

} // namespace graphview
