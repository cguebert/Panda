#include <panda/graphics/VertexArrayObject.h>

#include <GL/glew.h>

namespace panda
{

namespace graphics
{

struct VAOData
{
	~VAOData();

	unsigned int id = 0;
};

VAOData::~VAOData()
{
	glDeleteVertexArrays(1, &id);
}

//****************************************************************************//

void VertexArrayObject::create()
{
	if(!m_data)
		m_data = std::make_shared<VAOData>();

	if(!m_data->id)
		glGenVertexArrays(1, &m_data->id);
}

unsigned int VertexArrayObject::id() const
{
	if (m_data)
		return m_data->id;
	return 0;
}

void VertexArrayObject::bind() const
{
	if (m_data)
		glBindVertexArray(m_data->id);
}

void VertexArrayObject::release() const
{
	if(m_data)
		glBindVertexArray(0);
}

} // namespace graphics

} // namespace panda
