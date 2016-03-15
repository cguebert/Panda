#ifndef DOCUMENTSIGNALS_H
#define DOCUMENTSIGNALS_H

#include <panda/core.h>
#include <panda/messaging.h>

#include <panda/types/Point.h>

namespace panda {

class DockableObject;
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
	msg::Signal<void(const XmlElement&, panda::PandaObject*)> loadingObject;
	msg::Signal<void()> selectionChanged;
	msg::Signal<void()> timeChanged;
	msg::Signal<void()> reorderedObjects;
	msg::Signal<void()> startLoading;
	msg::Signal<void()> loadingFinished;
	msg::Signal<void(panda::DockableObject*)> changedDock;
	msg::Signal<void(int buttonId, bool isPressed, panda::types::Point localPos)> mouseButtonEvent;
	msg::Signal<void(panda::types::Point localPos, panda::types::Point globalPos)> mouseMoveEvent;
	msg::Signal<void(int key, bool isPressed)> keyEvent;
	msg::Signal<void(const std::string& text)> textEvent;
	msg::Signal<void(int w, int h, int fbo)> postRender;
};

} // namespace panda

#endif // DOCUMENTRENDERER_H
