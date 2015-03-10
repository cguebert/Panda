#include <panda/OGLObject.h>
#include <panda/PandaDocument.h>

namespace panda {

OGLObject::OGLObject(PandaDocument* document)
	: PandaObject(document)
	, m_isGLInitialized(false)
{
}

void OGLObject::updateIfDirty() const
{
	if(isDirty() && !m_isUpdating)
	{
		if(!m_isGLInitialized)
		{
			if(m_parentDocument->isGLInitialized())
			{
				const_cast<OGLObject*>(this)->initializeGL();
				const_cast<OGLObject*>(this)->m_isGLInitialized = true;
			}
			else
				return; // Cannot update this object yet as no valid OpenGL Context exists
		}

		PandaObject::updateIfDirty();
	}
}

} // namespace Panda


