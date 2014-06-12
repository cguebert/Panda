#include <panda/helper/GradientCache.h>
#include <QtGui/qopengl.h>

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
	for(auto item : m_cache)
		glDeleteTextures(1, &item.m_textureId);
	m_cache.clear();
}

void GradientCache::resetUsedFlag()
{
	for(auto& item : m_cache)
		item.m_used = false;
}

void GradientCache::clearUnused()
{
	GradientTableHash::iterator it = m_cache.begin(), itEnd = m_cache.end();
	while(it != itEnd)
	{
		const CacheItem& item = it.value();
		if(!item.m_used)
		{
			glDeleteTextures(1, &item.m_textureId);
			it = m_cache.erase(it);
		}
		else
			++it;
	}
}

unsigned int GradientCache::getTexture(const types::Gradient &gradient, int size)
{
	if(gradient.getStops().empty())
		return -1;
	quint64 hash = computeHash(gradient);
	GradientTableHash::iterator it = m_cache.find(hash), itEnd = m_cache.end();

	if(it == itEnd)
		return addGradient(hash, gradient, size);
	else
	{
		do {
			CacheItem& item = it.value();
			if(item.m_gradient == gradient && item.m_size >= size)
			{
				item.m_used = true;
				return item.m_textureId;
			}
			++it;
		} while(it != itEnd && it.key() == hash);
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

quint64 GradientCache::computeHash(const panda::types::Gradient& gradient)
{
	quint64 hash = 0;
	auto stops = gradient.getStops();
	for(int i=0, nb=stops.size(); i<nb && i<3; ++i)
		hash += stops[i].second.toHex();

	return hash;
}

unsigned int GradientCache::addGradient(quint64 hash, const types::Gradient &gradient, int size)
{
	size = qBound(64, nextPowerOf2(size), 1024);
	CacheItem item(gradient, size);

	auto buffer = createBuffer(gradient, size);
	glGenTextures(1, &item.m_textureId);
	glBindTexture(GL_TEXTURE_2D, item.m_textureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size, 1, 0, GL_RGBA, GL_FLOAT, buffer.constData());

	m_cache.insert(hash, item);
	return item.m_textureId;
}

QVector<types::Color> GradientCache::createBuffer(const panda::types::Gradient& gradient, int size)
{
	types::Gradient::GradientStops stops = gradient.getStops();
	int nbStops = stops.size();
	QVector<types::Color> buffer(size), colors(nbStops);

	for(int i=0; i<nbStops; ++i)
		colors[i] = stops[i].second;

	int pos = 0;
	types::Color prevColor = colors[0];
	buffer[pos++] = prevColor;
	PReal incr = 1.0 / PReal(size);
	PReal fpos = 1.5 * incr;

	while(fpos <= stops.first().first)
	{
		buffer[pos++] = buffer[0];
		fpos += incr;
	}

	for(int i=0; i<nbStops-1; ++i)
	{
		PReal prevPos = stops[i].first, nextPos = stops[i+1].first;
		PReal delta = 1/(nextPos - prevPos);
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
