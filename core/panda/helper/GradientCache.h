#ifndef HELPER_GRADIENTCACHE_H
#define HELPER_GRADIENTCACHE_H

#include <panda/core.h>
#include <panda/types/Gradient.h>

#include <unordered_map>

namespace panda
{

namespace helper
{

// Basically a copy of Qt's QGL2GradientCache
class PANDA_CORE_API GradientCache
{
public:
	static GradientCache* getInstance();

	void clear();			/// Remove all textures
	void resetUsedFlag();	/// Prepare the flags at the start of a render
	void clearUnused();		/// Remove textures not used during the last render

	unsigned int getTexture(const panda::types::Gradient& gradient, int size); /// Get the texture from the cache
	static std::vector<types::Color> createBuffer(const panda::types::Gradient& gradient, int size);

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

	typedef std::unordered_multimap<uint64_t, CacheItem> GradientTableHash;
	GradientTableHash m_cache;

	int nextPowerOf2(unsigned int v);
	static uint64_t computeHash(const panda::types::Gradient& gradient);
	unsigned int addGradient(uint64_t hash, const panda::types::Gradient& gradient, int size);
};

} // namespace helper

} // namespace panda

#endif // HELPER_GRADIENTCACHE_H
