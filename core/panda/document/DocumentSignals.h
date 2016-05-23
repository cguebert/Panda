#ifndef DOCUMENTSIGNALS_H
#define DOCUMENTSIGNALS_H

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
	msg::Signal<void()> timeChanged;
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
