#pragma once

#include <panda/core.h>

#include <memory>

namespace panda
{

namespace graphics
{

struct VAOData;

class PANDA_CORE_API VertexArrayObject
{
public:
	explicit operator bool() const; // Returns true if an OpenGL VAO has been created for this object
	bool isCreated() const;

	void create();
	void clear();

	unsigned int id() const;
	void bind() const;
	void release() const;

private:
	std::shared_ptr<VAOData> m_data;
};

inline VertexArrayObject::operator bool() const
{ return (id() != 0); }

inline bool VertexArrayObject::isCreated() const
{ return (id() != 0); }

inline void VertexArrayObject::clear()
{ m_data.reset(); }

} // namespace graphics

} // namespace panda
