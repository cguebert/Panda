#include <panda/graphics/Mat4x4.h>

#include <algorithm>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace panda
{

namespace graphics
{

void Mat4x4::setIdentity()
{
	std::fill(m_data, m_data + 16, 0.f);
	m_data[0] = m_data[5] = m_data[10] = m_data[15] = 1.f;
}

void Mat4x4::ortho(float left, float right, float bottom, float top, float near, float far)
{
	auto mat = glm::ortho(left, right, bottom, top, near, far);
	std::memcpy(m_data, glm::value_ptr(mat), sizeof(float) * 16);
}

void Mat4x4::scale(float x, float y, float z)
{
	glm::mat4x4 mat = glm::make_mat4x4(m_data);
	mat = glm::scale(mat, glm::vec3(x, y, z));
	std::memcpy(m_data, glm::value_ptr(mat), sizeof(float) * 16);
}

void Mat4x4::translate(float x, float y, float z)
{
	glm::mat4x4 mat = glm::make_mat4x4(m_data);
	mat = glm::translate(mat, glm::vec3(x, y, z));
	std::memcpy(m_data, glm::value_ptr(mat), sizeof(float) * 16);
}

void Mat4x4::rotate(float angle, float x, float y, float z)
{
	
	glm::mat4x4 mat = glm::make_mat4x4(m_data);
	mat = glm::rotate(mat, glm::radians(angle), glm::vec3(x, y, z));
	std::memcpy(m_data, glm::value_ptr(mat), sizeof(float) * 16);
}

} // namespace graphics

} // namespace panda

