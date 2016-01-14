#ifndef OGLOBJECT_H
#define OGLOBJECT_H

#include <panda/object/PandaObject.h>

namespace panda
{

// Inherit from this class for objects that use OpenGL commands
// The function initializeGL is called before any call to update (which is delayed until a valid context exists)
// The scheduler will not update this object in another thread
class PANDA_CORE_API OGLObject : public PandaObject
{
public:
	PANDA_CLASS(OGLObject, PandaObject)
	explicit OGLObject(PandaDocument* document);

	void updateIfDirty() const override;

	virtual void initializeGL() {}

protected:
	bool m_isGLInitialized;
};

//****************************************************************************//

} // namespace Panda

#endif // OGLOBJECT_H
