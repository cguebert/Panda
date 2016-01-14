#ifndef GRAPHICS_MAT4X4_H
#define GRAPHICS_MAT4X4_H

#include <panda/core.h>
#include <cmath>

namespace panda
{

namespace graphics
{

class PANDA_CORE_API Mat4x4
{
public:
	Mat4x4();

	void setIdentity();

	float* data();
	const float* data() const;

	void ortho(float left, float right, float bottom, float top, float near, float far);

	void scale(float x, float y, float z);
	void translate(float x, float y, float z);
	void rotate(float angle, float x, float y, float z);

protected:
	float m_data[16];
};

inline Mat4x4::Mat4x4()
{ setIdentity(); }

inline float* Mat4x4::data()
{ return m_data; }

inline const float* Mat4x4::data() const
{ return m_data; }

} // namespace graphics

} // namespace panda

#endif // GRAPHICS_MAT4X4_H
