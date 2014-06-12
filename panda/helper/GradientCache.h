#ifndef HELPER_GRADIENTCACHE_H
#define HELPER_GRADIENTCACHE_H

#include <QMultiHash>

#include <panda/types/Gradient.h>

namespace panda
{

namespace helper
{

// Basically a copy of Qt's QGL2GradientCache
class GradientCache
{
public:
	static GradientCache* getInstance();

	void clear();			/// Remove all textures
	void resetUsedFlag();	/// Prepare the flags at the start of a render
	void clearUnused();		/// Remove textures not used during the last render

	unsigned int getTexture(const panda::types::Gradient& gradient, int size); /// Get the texture from the cache
	static QVector<types::Color> createBuffer(const panda::types::Gradient& gradient, int size);

private:
	struct CacheItem
	{
		CacheItem(panda::types::Gradient g, int s)
			: m_gradient(g), m_size(s), m_used(true) {}

		panda::types::Gradient m_gradient;
		int m_size;
		unsigned int m_textureId;
		bool m_used;
	};

	typedef QMultiHash<quint64, CacheItem> GradientTableHash;
	GradientTableHash m_cache;

	int nextPowerOf2(unsigned int v);
	quint64 computeHash(const panda::types::Gradient& gradient);
	unsigned int addGradient(quint64 hash, const panda::types::Gradient& gradient, int size);
};

} // namespace helper

} // namespace panda

#endif // HELPER_GRADIENTCACHE_H
