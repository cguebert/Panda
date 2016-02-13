#include <panda/object/OGLObject.h>
#include <panda/PandaDocument.h>

namespace panda {

OGLObject::OGLObject(PandaDocument* document)
	: PandaObject(document)
	, m_isGLInitialized(false)
{
	setUpdateOnMainThread(true);
}

void OGLObject::updateIfDirty() const
{
	if(isDirty() && !isUpdating())
	{
		if(!m_isGLInitialized)
		{
			const_cast<OGLObject*>(this)->initializeGL();
			const_cast<OGLObject*>(this)->m_isGLInitialized = true;
		}

		PandaObject::updateIfDirty();
	}
}

} // namespace Panda


