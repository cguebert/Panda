#include <QtWidgets>
#include <functional>
#include <limits>

#include <ui/MainWindow.h>
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

GraphView::GraphView(panda::PandaDocument* doc, panda::ObjectsList& objectsList, MainWindow* mainWindow)
	: QOpenGLWidget(mainWindow)
	, m_pandaDocument(doc)
	, m_objectsList(objectsList)
	, m_viewRenderer(std::make_unique<ViewRenderer>())
	, m_viewGui(std::make_unique<ViewGui>(*this, mainWindow))
{
	QSurfaceFormat fmt;
	fmt.setSamples(8);
	setFormat(fmt);

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	setFocusPolicy(Qt::StrongFocus);

	setMouseTracking(true);

	const auto& pal = palette();
	m_drawColors.penColor = graphics::DrawList::convert(pal.text().color());
	m_drawColors.midLightColor = graphics::DrawList::convert(pal.midlight().color());
	m_drawColors.lightColor = graphics::DrawList::convert(pal.light().color());
	m_drawColors.highlightColor = graphics::DrawList::convert(pal.highlight().color());
}

GraphView::~GraphView() = default;

void GraphView::initComponents()
{
	if (!m_linksList)           m_linksList           = std::make_unique<LinksList>(*this);
	if (!m_linkTagsList)        m_linkTagsList        = std::make_unique<LinkTagsList>(*this);
	if (!m_objectsSelection)    m_objectsSelection    = std::make_unique<ObjectsSelection>(objectsList());
	if (!m_objectRenderersList) m_objectRenderersList = std::make_unique<ObjectRenderersList>();
	if (!m_viewport)            m_viewport            = std::make_unique<Viewport>(*this); 
	if (!m_interaction)         m_interaction         = std::make_unique<ViewInteraction>(*this);

	m_observer.get(selection().selectionChanged).connect<GraphView, &GraphView::selectionChanged>(this);

	auto& docSignals = m_pandaDocument->getSignals();
	m_observer.get(docSignals.modified).connect<QWidget, &QWidget::update>(this);
	m_observer.get(docSignals.modifiedObject).connect<GraphView, &GraphView::modifiedObject>(this);
	m_observer.get(docSignals.startLoading).connect<GraphView, &GraphView::startLoading>(this);
	m_observer.get(docSignals.loadingFinished).connect<GraphView, &GraphView::loadingFinished>(this);
	m_observer.get(docSignals.changedDock).connect<GraphView, &GraphView::changedDock>(this);

	m_observer.get(m_objectsList.addedObject).connect<GraphView, &GraphView::addedObject>(this);
	m_observer.get(m_objectsList.removedObject).connect<GraphView, &GraphView::removeObject>(this);
	m_observer.get(m_objectsList.reorderedObjects).connect<GraphView, &GraphView::objectsReordered>(this);

	m_observer.get(viewport().modified).connect<QWidget, &QWidget::update>(this);
	m_observer.get(viewport().modified).connect<GraphView, &GraphView::emitViewportModified>(this);

	// Create the draw structs for the objects already present
	for (const auto& object : m_objectsList.get())
		addedObject(object.get());

	updateDirtyRenderers();
	viewport().updateObjectsRect();
}

QSize GraphView::minimumSizeHint() const
{
	return QSize(300, 200);
}

QSize GraphView::sizeHint() const
{
	return QSize(600, 400);
}

void GraphView::initializeGL()
{
	m_viewRenderer->initialize();
	initializeRenderer(*m_viewRenderer);
}

void GraphView::resizeGL(int w, int h)
{
	viewport().setViewSize({ static_cast<float>(w), static_cast<float>(h) });
	m_viewRenderer->resize(w, h);
	update();
}

void GraphView::paintGL()
{
	decltype(m_functionsToExecuteNextRefresh) functions;
	functions.swap(m_functionsToExecuteNextRefresh);
	for (const auto func : functions)
		func();

	beforeDraw();

	// Prepare the renderer
	const auto displayRect = viewport().displayRect();
	m_viewRenderer->setView(displayRect);
	m_viewRenderer->newFrame();

	drawGraphView(*m_viewRenderer, m_drawColors);

	// Execute the render commands
	m_viewRenderer->render();
}

void GraphView::beforeDraw()
{
	interaction().beforeDraw();

	updateDirtyRenderers();

	if (m_objectsMoved)
	{
		m_objectsMoved = false;
		emit modified();
		linkTagsList().setDirty();
		linksList().clear();
		viewport().updateObjectsRect();
	}
}

void GraphView::initializeRenderer(ViewRenderer& viewRenderer)
{
	m_drawList = std::make_shared<graphics::DrawList>(viewRenderer);
	interaction().initializeRenderer(viewRenderer);
	linksList().initializeRenderer(viewRenderer);
	objectRenderers().initializeRenderer(viewRenderer);
}

void GraphView::drawGraphView(ViewRenderer& viewRenderer, graphics::DrawColors drawColors)
{
	const auto displayRect = viewport().displayRect();

	linkTagsList().onBeginDraw();
	linksList().onBeginDraw(drawColors);
	interaction().onBeginDraw(drawColors);

	auto& drawList = *m_drawList;
	drawList.clear();

	auto col = palette().background().color();
	glClearColor(col.redF(), col.greenF(), col.blueF(), 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	const auto& orderedObjectRenderers = objectRenderers().getOrdered();

	// Give a possibility to draw behind normal objects
	for (auto& objRnd : orderedObjectRenderers)
	{
		if (objRnd->getVisualArea().intersects(displayRect))
			objRnd->drawBackground(drawList, drawColors);
	}

	// Draw links
	drawList.merge(*linksList().linksDrawList());

	// Draw the objects
	for (auto& objRnd : orderedObjectRenderers)
	{
		if (objRnd->getVisualArea().intersects(displayRect))
			objRnd->draw(drawList, drawColors);
	}

	// Give a possibility to draw in front of normal objects
	for (auto& objRnd : orderedObjectRenderers)
	{
		if (objRnd->getVisualArea().intersects(displayRect))
			objRnd->drawForeground(drawList, drawColors);
	}

	// Redraw selected objets in case they are moved over others (so that they don't appear under them)
	for (auto& objRnd : m_selectedObjectsRenderers)
	{
		if (objRnd->getVisualArea().intersects(displayRect))
			objRnd->draw(drawList, drawColors, true);
	}

	linkTagsList().drawTags(drawList, drawColors);
	interaction().drawInteraction(drawList, drawColors);

	if (m_debugDirtyState)
	{
#ifdef PANDA_LOG_EVENTS
		UpdateLoggerDialog* logDlg = UpdateLoggerDialog::getInstance();
		if(logDlg && logDlg->isVisible())
			paintLogDebug(drawList, drawColors);
		else
#endif
			paintDirtyState(drawList, drawColors);
	}

	// Add the main draw list
	viewRenderer.addDrawList(m_drawList);

	// Highlight connected Datas
	if(interaction().highlightConnectedDatas())
		viewRenderer.addDrawList(interaction().connectedDatasDrawList());
}

#ifdef PANDA_LOG_EVENTS
void GraphView::paintLogDebug(graphics::DrawList& list, graphics::DrawColors& colors)
{
	UpdateLoggerDialog* logDlg = UpdateLoggerDialog::getInstance();
	if(logDlg && logDlg->isVisible())
	{
		const auto& states = logDlg->getNodeStates();
		for(const auto objRnd : objectRenderers().getOrdered())
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
			panda::PandaObject* object = m_objectsList.find(event->m_objectIndex);
			if(object)
			{
				auto objRnd = objectRenderers().get(object);
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

void GraphView::paintDirtyState(graphics::DrawList& list, graphics::DrawColors& colors)
{
	for(const auto& objRnd : objectRenderers().getOrdered())
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

void GraphView::mousePressEvent(QMouseEvent* event)
{
	interaction().mousePressEvent({ convert(event->pos()), convert(event->button()), convert(event->modifiers()) });
}

void GraphView::mouseMoveEvent(QMouseEvent* event)
{
	interaction().mouseMoveEvent({ convert(event->pos()), convert(event->button()), convert(event->modifiers()) });
}

void GraphView::mouseReleaseEvent(QMouseEvent* event)
{
	interaction().mouseReleaseEvent({ convert(event->pos()), convert(event->button()), convert(event->modifiers()) });
}

void GraphView::wheelEvent(QWheelEvent* event)
{
	interaction().wheelEvent({ convert(event->pos()), convert(event->angleDelta()), convert(event->modifiers()) });
}

void GraphView::keyPressEvent(QKeyEvent* event)
{
	KeyEvent ke { static_cast<Key>(event->key()), convert(event->modifiers()) };
	if (!interaction().keyPressEvent(ke))
	{
		if(event->key() == Qt::Key_Space && !document()->animationIsPlaying())
			QuickCreateDialog { document(), this }.exec();
		else
			QWidget::keyPressEvent(event);
	}
}

void GraphView::contextMenuEvent(QContextMenuEvent* event)
{
	interaction().contextMenuEvent({ convert(event->pos()), convert(event->modifiers()) });
}

void GraphView::addedObject(panda::PandaObject* object)
{
	// Creating a Renderer depending on the class of the object being added
	auto objRnd = objectRenderers().get(object);
	if (!objRnd)
	{
		auto objRndPtr = object::ObjectRendererFactory::getInstance()->createRenderer(this, object);
		
		if (m_viewRenderer->initialized())
			objRndPtr->initializeRenderer(*m_viewRenderer);

		objectRenderers().set(object, objRndPtr);
		objRnd = objRndPtr.get();
	}

	objRnd->setDirty();
	update();
}

void GraphView::removeObject(panda::PandaObject* object)
{
	objectRenderers().remove(object);
	interaction().removeObject(object);
	linkTagsList().clear();
	linksList().clear();

	viewport().updateObjectsRect();
}

void GraphView::modifiedObject(panda::PandaObject* object)
{
	linksList().clear();
	auto objRnd = objectRenderers().get(object);
	if(objRnd)	// Can be called before the object is fully created
	{
		objRnd->setDirty();

		panda::DockObject* dock = dynamic_cast<panda::DockObject*>(object);
		if (dock)
		{
			auto dobjRnd = dynamic_cast<object::DockObjectRenderer*>(objRnd);
			if (dobjRnd)
				dobjRnd->placeDockableObjects();
		}

		update();
	}
}

void GraphView::objectsMoved()
{
	m_objectsMoved = true;
	update();
}

void GraphView::sortDockable(panda::DockableObject* dockable, panda::DockObject* defaultDock)
{
	int prevIndex = defaultDock->getIndexOfDockable(dockable);
	auto dockables = defaultDock->getDockedObjects();

	std::sort(dockables.begin(), dockables.end(), [this](panda::DockableObject* lhs, panda::DockableObject* rhs){
		auto lpos = objectRenderers().get(lhs)->getPosition();
		auto rpos = objectRenderers().get(rhs)->getPosition();
		if(lpos.y == rpos.y)
			return lpos.x > rpos.x;
		return lpos.y < rpos.y;
	});

	auto iter = std::find(dockables.begin(), dockables.end(), dockable);
	if(iter != dockables.end())
	{
		int newIndex = iter - dockables.begin();
		if(newIndex == prevIndex)
			return;

		m_pandaDocument->getUndoStack().push(std::make_shared<panda::ReorderDockableCommand>(defaultDock, dockable, newIndex));
	}
}

void GraphView::sortDockablesInDock(panda::DockObject* dock)
{
	for(const auto dockable : dock->getDockedObjects())
	{
		auto defaultDock = dockable->getDefaultDock();
		auto parentDock = dockable->getParentDock();
		if(defaultDock && parentDock == defaultDock)
			sortDockable(dockable, defaultDock);
	}
}

void GraphView::sortAllDockables()
{
	for(const auto& object : m_objectsList.get())
	{
		const auto dockable = dynamic_cast<panda::DockableObject*>(object.get());
		if(!dockable)
			continue;
		auto defaultDock = dockable->getDefaultDock();
		auto parentDock = dockable->getParentDock();
		if(defaultDock && parentDock == defaultDock)
			sortDockable(dockable, defaultDock);
	}
}

void GraphView::startLoading()
{
	m_isLoading = true;
}

void GraphView::loadingFinished()
{
	m_isLoading = false;
	sortAllDockables();
	updateDirtyRenderers();
}

void GraphView::changedDock(panda::DockableObject* dockable)
{
	auto defaultDock = dockable->getDefaultDock();
	auto parentDock = dockable->getParentDock();
	if(defaultDock && parentDock == defaultDock)
		sortDockable(dockable, defaultDock);
}

QSize GraphView::viewSize()
{
	const auto& viewRect = viewport().viewRect();
	return QSize(viewRect.width(), viewRect.height());
}

QPoint GraphView::viewPosition()
{
	const auto& viewRect = viewport().viewRect();
	auto delta = viewport().viewDelta() * viewport().zoom();
	return QPoint(viewRect.left() - delta.x, viewRect.top() - delta.y);
}

void GraphView::scrollView(QPoint position)
{
	const auto& viewRect = viewport().viewRect();
	Point delta = convert(position) - viewRect.topLeft() + viewport().viewDelta() * viewport().zoom();
	viewport().moveView(delta);
}

void GraphView::showChooseWidgetDialog()
{
	auto contextMenuData = interaction().contextMenuData();
	if (contextMenuData)
		ChooseWidgetDialog(contextMenuData, this).exec();
	else
	{
		auto obj = selection().lastSelectedObject();
		if(obj && obj->getClass()->getClassName() == "GeneratorUser" && obj->getClass()->getNamespaceName() == "panda")
		{
			auto data = obj->getData("input");
			if(data)
				ChooseWidgetDialog(data, this).exec();
		}
	}
}

void GraphView::focusOutEvent(QFocusEvent*)
{
	emit lostFocus(this);
}

void GraphView::selectionChanged()
{
	m_selectedObjectsRenderers = objectRenderers().get(selection().get());
	update();
}

void GraphView::updateDirtyRenderers()
{
	bool updated = false;

	// Bugfix: update the dock objects last
	const auto& orderedObjectRenderers = objectRenderers().getOrdered();
	for (auto objRnd : orderedObjectRenderers)
		if(!dynamic_cast<panda::DockObject*>(objRnd->getObject()))
			updated |= objRnd->updateIfDirty();

	for (auto objRnd : orderedObjectRenderers)
		if(dynamic_cast<panda::DockObject*>(objRnd->getObject()))
			updated |= objRnd->updateIfDirty();

	if (!updated)
		return;

	linksList().clear();
	linkTagsList().clear();
	interaction().clear();
	viewport().updateObjectsRect();
}

void GraphView::objectsReordered()
{
	objectRenderers().reorder(m_objectsList);
}

void GraphView::setDataLabel()
{
	auto data = interaction().contextMenuData();
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
void GraphView::copy()
{
	const auto& selected = selection().get();
	if (selected.empty())
		return;

	QApplication::clipboard()->setText(QString::fromStdString(panda::serialization::writeTextDocument(m_pandaDocument, selected)));
}

void GraphView::cut()
{
	copy();
	del();
}

void GraphView::paste()
{
	const QMimeData* mimeData = QApplication::clipboard()->mimeData();
	if (!mimeData->hasText())
		return;
	
	auto result = panda::serialization::readTextDocument(m_pandaDocument, objectsList(), mimeData->text().toStdString());
	if (!result.first || result.second.empty())
		return;

	selection().set(result.second);
	viewport().moveSelectedToCenter();

	m_pandaDocument->getUndoStack().push(std::make_shared<AddObjectCommand>(m_pandaDocument, m_objectsList, result.second));
}

void GraphView::del()
{
	const auto& selected = selection().get();
	if (selected.empty())
		return;

	auto macro = m_pandaDocument->getUndoStack().beginMacro(tr("delete objects").toStdString());	
	m_pandaDocument->getUndoStack().push(std::make_shared<RemoveObjectCommand>(m_pandaDocument, m_objectsList, selected));
}

void GraphView::executeNextRefresh(std::function<void()> func)
{
	m_functionsToExecuteNextRefresh.push_back(func);
}

void GraphView::emitViewportModified()
{
	emit viewportModified();
}

void GraphView::setGui(const std::shared_ptr<ViewRenderer>& viewRenderer, std::unique_ptr<ViewGui> viewGui)
{
	m_viewRenderer = viewRenderer;
	m_viewGui = std::move(viewGui);
}

} // namespace graphview
