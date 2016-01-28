#pragma once

#include <panda/core.h>
#include <memory>

namespace panda
{

namespace graphics
{

enum class BufferType : unsigned int
{
	ArrayBuffer			= 0x8892, // GL_ARRAY_BUFFER
	ElementArrayBuffer	= 0x8893, // GL_ELEMENT_ARRAY_BUFFER
	PixelPackBuffer		= 0x88EB, // GL_PIXEL_PACK_BUFFER
	PixelUnpackBuffer	= 0x88EC  // GL_PIXEL_UNPACK_BUFFER
};

enum class BufferUsage : unsigned int
{
	StreamDraw	= 0x88E0, // GL_STREAM_DRAW
	StreamRead	= 0x88E1, // GL_STREAM_READ
	StreamCopy	= 0x88E2, // GL_STREAM_COPY
	StaticDraw	= 0x88E4, // GL_STATIC_DRAW
	StaticRead	= 0x88E5, // GL_STATIC_READ
	StaticCopy	= 0x88E6, // GL_STATIC_COPY
	DynamicDraw	= 0x88E8, // GL_DYNAMIC_DRAW
	DynamicRead	= 0x88E9, // GL_DYNAMIC_READ
	DynamicCopy	= 0x88EA  // GL_DYNAMIC_COPY
};

struct BufferData;

class PANDA_CORE_API Buffer
{
public:
	Buffer() = default;

	explicit operator bool() const; // Returns true if an OpenGL buffer has been created for this object
	bool isCreated() const;

	void create(BufferType type = BufferType::ArrayBuffer, BufferUsage usage = BufferUsage::StaticDraw);
	void clear();

	BufferType type() const;
	BufferUsage usage() const;
	void setUsage(BufferUsage usage);

	unsigned int id() const;
	void bind() const;
	void release() const;
	static void release(BufferType type);

	int size() const;

	// For the following methods, we assume the buffer has been created and is currently bound to the context
	void write(int offset, const void* data, int count); // Automatically calls allocate if count > current size

    void allocate(const void* data, int count);
	void allocate(int count);

	template <typename T>
	void write(int offset, const std::vector<T>& v)
	{ write(offset, v.data(), sizeof(T) * v.size()); }

	template <typename T, size_t N>
	void write(int offset, const std::vector<std::array<T, N>>& v)
	{ write(offset, v.data(), sizeof(T) * N * v.size()); }

	template <typename T>
	void write(const std::vector<T>& v)
	{ write(0, v.data(), sizeof(T) * v.size()); }

	template <typename T, size_t N>
	void write(const std::vector<std::array<T, N>>& v)
	{ write(0, v.data(), sizeof(T) * N * v.size()); }

	template <typename T>
	void allocate(const std::vector<T>& v)
	{ allocate(v.data(), sizeof(T) * v.size()); }

	template <typename T, size_t N>
	void allocate(const std::vector<std::array<T, N>>& v)
	{ allocate(v.data(), sizeof(T) * N * v.size()); }

private:
	std::shared_ptr<BufferData> m_data;
};

inline Buffer::operator bool() const
{ return (id() != 0); }

inline bool Buffer::isCreated() const
{ return (id() != 0); }

inline void Buffer::clear()
{ m_data.reset(); }

inline void Buffer::allocate(int count) 
{ allocate(nullptr, count); }

} // namespace graphics

} // namespace panda
