#include <ui/graphview/QtViewWrapper.h>

#include <QtWidgets>
#include <functional>
#include <limits>

#include <ui/MainWindow.h>
#include <ui/command/MoveObjectCommand.h>
#include <ui/dialog/ChooseWidgetDialog.h>
#include <ui/dialog/QuickCreateDialog.h>
#include <ui/graphview/object/ObjectRenderer.h>
#include <ui/graphview/object/DockableRenderer.h>
#include <ui/graphview/DataLabelAddon.h>
#include <ui/graphview/GraphView.h>
#include <ui/graphview/LinksList.h>
#include <ui/graphview/LinkTagsList.h>
#include <ui/graphview/ObjectsSelection.h>
#include <ui/graphview/ObjectRenderersList.h>
#include <ui/graphview/ViewGUI.h>
#include <ui/graphview/ViewInteraction.h>
#include <ui/graphview/Viewport.h>
#include <ui/graphview/ViewRenderer.h>
#include <ui/graphview/graphics/DrawList.h>

#include <panda/document/PandaDocument.h>
#include <panda/SimpleGUI.h>
#include <panda/helper/algorithm.h>
#include <panda/command/AddObjectCommand.h>
#include <panda/command/DockableCommand.h>
#include <panda/command/RemoveObjectCommand.h>
#include <panda/document/DocumentSignals.h>
#include <panda/document/ObjectsList.h>
#include <panda/document/Serialization.h>

#ifdef PANDA_LOG_EVENTS
#include <ui/dialog/UpdateLoggerDialog.h>
#endif

using Point = panda::types::Point;
using Rect = panda::types::Rect;

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

	graphview::EventModifiers convert(Qt::KeyboardModifiers modifiers)
	{
		graphview::EventModifiers mod = graphview::EventModifier::NoModifier;
		if (modifiers & Qt::KeyboardModifier::ShiftModifier)   mod |= graphview::EventModifier::ShiftModifier;
		if (modifiers & Qt::KeyboardModifier::ControlModifier) mod |= graphview::EventModifier::ControlModifier;
		if (modifiers & Qt::KeyboardModifier::AltModifier)     mod |= graphview::EventModifier::AltModifier;

		return mod;
	}

	graphview::MouseButton convert(Qt::MouseButton button)
	{
		if (button == Qt::MouseButton::LeftButton)   return graphview::MouseButton::LeftButton;
		if (button == Qt::MouseButton::RightButton)  return graphview::MouseButton::RightButton;
		if (button == Qt::MouseButton::MiddleButton) return graphview::MouseButton::MiddleButton;

		return graphview::MouseButton::NoButton;
	}
}

namespace graphview
{

QtViewWrapper::QtViewWrapper(std::unique_ptr<GraphView> graphView, MainWindow* mainWindow)
	: QOpenGLWidget(mainWindow)
	, m_graphView(std::move(graphView))
	, m_viewRenderer(std::make_shared<ViewRenderer>())
{
	QSurfaceFormat fmt;
	fmt.setSamples(8);
	setFormat(fmt);

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	setFocusPolicy(Qt::StrongFocus);

	setMouseTracking(true);

	auto viewGui = std::make_unique<ViewGui>(*this, mainWindow);

	const auto& pal = palette();
	m_drawColors.penColor = graphics::DrawList::convert(pal.text().color());
	m_drawColors.midLightColor = graphics::DrawList::convert(pal.midlight().color());
	m_drawColors.lightColor = graphics::DrawList::convert(pal.light().color());
	m_drawColors.highlightColor = graphics::DrawList::convert(pal.highlight().color());
	m_drawColors.backgroundColor = graphics::DrawList::convert(pal.highlight().color());

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
	m_drawList = std::make_shared<graphics::DrawList>(*m_viewRenderer);
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
void QtViewWrapper::paintLogDebug(graphics::DrawList& list, graphics::DrawColors& colors)
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

void QtViewWrapper::paintDirtyState(graphics::DrawList& list, graphics::DrawColors& colors)
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
	KeyEvent ke { static_cast<Key>(event->key()), convert(event->modifiers()) };
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

	auto label = DataLabelAddon::getDataLabel(data);

	bool ok = false;
	label = QInputDialog::getMultiLineText(this, tr("Data label"), tr("Label:"), QString::fromStdString(label), &ok).toStdString();
	if (!ok)
		return;

	DataLabelAddon::setDataLabel(data, label);

	emit modified();
}
void QtViewWrapper::copy()
{
	const auto& selected = m_graphView->selection().get();
	if (selected.empty())
		return;

	QApplication::clipboard()->setText(QString::fromStdString(panda::serialization::writeTextDocument(m_graphView->document(), selected)));
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
	
	auto result = panda::serialization::readTextDocument(m_graphView->document(), m_graphView->objectsList(), mimeData->text().toStdString());
	if (!result.first || result.second.empty())
		return;

	m_graphView->selection().set(result.second);
	m_graphView->viewport().moveSelectedToCenter();

	m_graphView->document()->getUndoStack().push(std::make_shared<AddObjectCommand>(m_graphView->document(), m_graphView->objectsList(), result.second));
}

void QtViewWrapper::del()
{
	const auto& selected = m_graphView->selection().get();
	if (selected.empty())
		return;

	auto macro = m_graphView->document()->getUndoStack().beginMacro(tr("delete objects").toStdString());	
	m_graphView->document()->getUndoStack().push(std::make_shared<RemoveObjectCommand>(m_graphView->document(), m_graphView->objectsList(), selected));
}

void QtViewWrapper::executeNextRefresh(std::function<void()> func)
{
	m_functionsToExecuteNextRefresh.push_back(func);
}

void QtViewWrapper::emitViewportModified()
{
	emit viewportModified();
}

} // namespace graphview
