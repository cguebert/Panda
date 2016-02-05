#ifndef DOCUMENTSIGNALS_H
#define DOCUMENTSIGNALS_H

#include <panda/core.h>
#include <panda/messaging.h>

#include <panda/types/Point.h>

namespace panda {

class PandaObject;
class XmlElement;

class PANDA_CORE_API DocumentSignals
{
public:
	msg::Signal<void()> modified;
	msg::Signal<void(panda::PandaObject*)> modifiedObject;
	msg::Signal<void(panda::PandaObject*)> dirtyObject;
	msg::Signal<void(panda::PandaObject*)> addedObject;
	msg::Signal<void(panda::PandaObject*)> removedObject;
	msg::Signal<void(panda::PandaObject*)> selectedObject;
	msg::Signal<void(panda::PandaObject*)> selectedObjectIsDirty;
	msg::Signal<void(XmlElement&, panda::PandaObject*)> savingObject;
	msg::Signal<void(XmlElement&, panda::PandaObject*)> loadingObject;
	msg::Signal<void()> selectionChanged;
	msg::Signal<void()> timeChanged;
	msg::Signal<void()> renderSizeChanged;
	msg::Signal<void()> reorderedObjects;
	msg::Signal<void()> startLoading;
	msg::Signal<void()> loadingFinished;
	msg::Signal<void(panda::DockableObject*)> changedDock;
	msg::Signal<void(panda::types::Point)> mousePressed;
	msg::Signal<void(panda::types::Point)> mouseReleased;
};

} // namespace panda

#endif // DOCUMENTRENDERER_H
