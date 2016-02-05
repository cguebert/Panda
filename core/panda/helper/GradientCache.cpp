#include <GL/glew.h>

#include <panda/helper/GradientCache.h>

#include <algorithm>
#include <vector>

namespace panda
{

namespace helper
{

GradientCache* GradientCache::getInstance()
{
	static GradientCache instance;
	return &instance;
}

void GradientCache::clear()
{
	for(const auto& item : m_cache)
		glDeleteTextures(1, &item.second.m_textureId);
	m_cache.clear();
}

void GradientCache::resetUsedFlag()
{
	for(auto& item : m_cache)
		item.second.m_used = false;
}

void GradientCache::clearUnused()
{
	GradientTableHash::iterator it = m_cache.begin(), itEnd = m_cache.end();
	while(it != itEnd)
	{
		const CacheItem& item = it->second;
		if(!item.m_used)
		{
			glDeleteTextures(1, &item.m_textureId);
			it = m_cache.erase(it);
		}
		else
			++it;
	}
}

unsigned int GradientCache::getTexture(const types::Gradient& gradient, int size)
{
	if(gradient.getStops().empty())
		return 0;
	uint64_t hash = computeHash(gradient);
	GradientTableHash::iterator it = m_cache.find(hash), itEnd = m_cache.end();

	if(it == itEnd)
		return addGradient(hash, gradient, size);
	else
	{
		do {
			CacheItem& item = it->second;
			if(item.m_gradient == gradient && item.m_size >= size)
			{
				item.m_used = true;
				return item.m_textureId;
			}
			++it;
		} while(it != itEnd && it->first == hash);
		return addGradient(hash, gradient, size);
	}
}

int GradientCache::nextPowerOf2(unsigned int v)
{
	// From http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;

	return v;
}

uint64_t GradientCache::computeHash(const panda::types::Gradient& gradient)
{
	uint64_t hash = 0;
	auto stops = gradient.getStops();
	for(int i=0, nb=stops.size(); i<nb && i<3; ++i)
		hash += static_cast<uint64_t>(stops[i].second.toHex() * stops[i].first);

	return hash;
}

unsigned int GradientCache::addGradient(uint64_t hash, const types::Gradient &gradient, int size)
{
	size = std::max(64, std::min(nextPowerOf2(size), 1024));
	CacheItem item(gradient, size);

	auto buffer = createBuffer(gradient, size);
	glGenTextures(1, &item.m_textureId);
	glBindTexture(GL_TEXTURE_2D, item.m_textureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size, 1, 0, GL_RGBA, GL_FLOAT, buffer.data());

	m_cache.emplace(hash, item);
	return item.m_textureId;
}

std::vector<types::Color> GradientCache::createBuffer(const panda::types::Gradient& gradient, int size)
{
	types::Gradient::GradientStops stops = gradient.getStops();
	int nbStops = stops.size();
	std::vector<types::Color> buffer(size), colors(nbStops);

	for(int i=0; i<nbStops; ++i)
		colors[i] = stops[i].second;

	int pos = 0;
	types::Color prevColor = colors[0];
	buffer[pos++] = prevColor;
	float incr = 1.0f / size;
	float fpos = 1.5f * incr;

	while(fpos <= stops.front().first)
	{
		buffer[pos++] = buffer[0];
		fpos += incr;
	}

	for(int i=0; i<nbStops-1; ++i)
	{
		float prevPos = stops[i].first, nextPos = stops[i+1].first;
		float delta = 1/(nextPos - prevPos);
		types::Color nextColor = colors[i+1];
		while(fpos < nextPos && pos < size)
		{
			buffer[pos++] = types::Gradient::interpolate(prevColor, nextColor, (fpos - prevPos) * delta);
			fpos += incr;
		}
		prevColor = nextColor;
	}

	while(pos < size)
		buffer[pos++] = prevColor;

	return buffer;
}

} // namespace helper

} // namespace panda
