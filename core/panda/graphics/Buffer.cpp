#include <panda/graphics/Buffer.h>

#include <GL/glew.h>

namespace
{
	inline GLenum toGL(panda::graphics::BufferType type)
	{ return static_cast<GLenum>(type); }

	inline GLenum toGL(panda::graphics::BufferUsage usage)
	{ return static_cast<GLenum>(usage); }
}

namespace panda
{

namespace graphics
{

struct BufferData
{
	~BufferData();

	unsigned int id = 0;
	BufferType type = BufferType::ArrayBuffer;
	BufferUsage usage = BufferUsage::StaticDraw;
};

BufferData::~BufferData()
{
	glDeleteBuffers(1, &id);
}

//****************************************************************************//

void Buffer::create(BufferType type, BufferUsage usage)
{
	// First clean up if something has already been created
	release();
	m_data.reset();

	m_data = std::make_shared<BufferData>();
	m_data->type = type;
	m_data->usage = usage;

	glGenBuffers(1, &m_data->id);
}

BufferType Buffer::type() const
{
	if (m_data)
		return m_data->type;
	return BufferType::ArrayBuffer;
}

BufferUsage Buffer::usage() const
{
	if (m_data)
		return m_data->usage;
	return BufferUsage::StaticDraw;
}

void Buffer::setUsage(BufferUsage usage)
{
	if (m_data)
		m_data->usage = usage;
}

unsigned int Buffer::id() const
{
	if (m_data)
		return m_data->id;
	return 0;
}

void Buffer::bind() const
{
	if (m_data)
		glBindBuffer(toGL(m_data->type), m_data->id);
}

void Buffer::release() const
{
	if(m_data)
		glBindBuffer(toGL(m_data->type), 0);
}

void Buffer::release(BufferType type)
{
	glBindBuffer(toGL(type), 0);
}

void Buffer::write(int offset, const void* data, int count)
{
	if (m_data)
		glBufferSubData(toGL(m_data->type), offset, count, data);
}

void Buffer::allocate(const void* data, int count)
{
	if (m_data)
		glBufferData(toGL(m_data->type), count, data, toGL(m_data->usage));
}

} // namespace graphics

} // namespace panda
