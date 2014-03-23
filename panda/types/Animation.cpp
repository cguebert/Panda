#include <panda/types/Animation.inl>
#include <panda/types/AnimationTraits.h>

#include <panda/DataFactory.h>
#include <panda/Data.inl>

#if QT_POINTER_SIZE == 8 // 64-bit versions

static inline uint interpolateColor(uint x, uint a, uint y, uint b) {
	quint64 t = (((quint64(x)) | ((quint64(x)) << 24)) & 0x00ff00ff00ff00ff) * a;
	t += (((quint64(y)) | ((quint64(y)) << 24)) & 0x00ff00ff00ff00ff) * b;
	t >>= 8;
	t &= 0x00ff00ff00ff00ff;
	return (uint(t)) | (uint(t >> 24));
}

#else // 32-bit versions

static inline uint interpolateColor(uint x, uint a, uint y, uint b) {
	uint t = (x & 0xff00ff) * a + (y & 0xff00ff) * b;
	t >>= 8;
	t &= 0xff00ff;

	x = ((x >> 8) & 0xff00ff) * a + ((y >> 8) & 0xff00ff) * b;
	x &= 0xff00ff00;
	return (x | t);
}

#endif

namespace panda
{

namespace types
{

template<>
QColor interpolate(const QColor& v1, const QColor& v2, PReal amt)
{
	int dist = static_cast<int>(256 * amt);
	int idist = 256 - dist;
	return QColor::fromRgba(interpolateColor(v1.rgba(), dist, v2.rgba(), idist));
}

template class Animation<PReal>;
template class Animation<QColor>;

template class Data< Animation<PReal> >;
template class Data< Animation<QColor> >;

int realAnimationDataClass = RegisterData< Animation<PReal> >();
int colorAnimationDataClass = RegisterData< Animation<QColor> >();

} // namespace types

} // namespace panda
