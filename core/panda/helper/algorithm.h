#ifndef HELPER_ALGORITHM_H
#define HELPER_ALGORITHM_H

#include <algorithm>

namespace panda
{

namespace helper
{

template <class T>
const T& bound(const T& min, const T& value, const T& max)
{
	return std::max(min, std::min(value, max));
}

template <class Container, class Value>
bool contains(const Container& container, const Value& value)
{
	return std::find(std::begin(container), std::end(container), value) != std::end(container);
}

template <class Container, class Pred>
bool contains_if(const Container& container, const Pred& pred)
{
	return std::find_if(std::begin(container), std::end(container), pred) != std::end(container);
}

template <class Container, class Value>
bool find(const Container& container, const Value& value)
{
	return std::find(std::begin(container), std::end(container), value);
}

template <class Container, class Pred>
bool find_if(const Container& container, const Pred& pred)
{
	return std::find_if(std::begin(container), std::end(container), pred);
}

template <class Container, class Value>
int count(const Container& container, const Value& value)
{
	return std::count(std::begin(container), std::end(container), value);
}

template <class Container, class Value>
int indexOf(const Container& container, const Value& value)
{
	auto it = std::find(std::begin(container), std::end(container), value);
	if (it == std::end(container))
		return -1;
	return std::distance(std::begin(container), it);
}

template <class Container, class Value>
int removeAll(Container& container, const Value& value)
{
	if (container.empty())
		return 0;
	auto last = std::remove(container.begin(), container.end(), value);
	int dist = std::distance(last, container.end());
	container.erase(last, container.end());
	return dist;
}

template <class Container, class Value>
bool removeOne(Container& container, const Value& value)
{
	auto it = std::find(container.begin(), container.end(), value);
	if (it == container.end())
		return false;

	container.erase(it);
	return true;
}

template <class Container, class Function>
int removeIf(Container& container, const Function& function)
{
	auto last = std::remove_if(container.begin(), container.end(), function);
	int dist = std::distance(last, container.end());
	container.erase(last, container.end());
	return dist;
}

template <class Container>
void removeAt(Container& container, unsigned int index)
{
	container.erase(container.begin() +  index);
}

template<class Container, class Indices>
void removeIndices(Container& container, const Indices& indices)
{
	typedef typename Container::iterator iterator;
	iterator first = std::begin(container);
	iterator last = std::end(container);

	typedef typename Indices::const_iterator indices_iterator;
	indices_iterator firstIndex = std::begin(indices);
	indices_iterator lastIndex = std::end(indices);

	iterator result = first;
	for (int i=0; first != last; ++first, ++i)
	{
		if (firstIndex == lastIndex || *firstIndex != i)
			*result++ = *first;
		else
			++firstIndex;
	}

	container.erase(result, last);
}

template <class Container1, class Container2>
void concatenate(Container1& to, const Container2& from)
{
	to.insert(to.end(), from.begin(), from.end());
}

template <class Container, class Key>
typename Container::mapped_type valueOrDefault(Container& map, const Key& key, const typename Container::mapped_type& defaultValue = typename Container::mapped_type())
{
	using value_type = typename Container::mapped_type;
	auto it = map.find(key);
	if (it == map.end())
		return defaultValue;
	return it->second;
}

template <class String>
void replaceAll(String& val, const String& from, const String& to)
{
	if (from.empty())
		return;

	auto fromSize = from.size(), toSize = to.size();
	size_t pos = 0;
	while ((pos = val.find(from, pos)) != String::npos)
	{
		val.replace(pos, fromSize, to);
		pos += toSize;
	}
}

template <class It> 
std::pair<It, It> slide(It first, It last, It pos)
{
	if (pos < first) 
		return { pos, std::rotate(pos, first, last) };
	if (last < pos)
		return { std::rotate(first, last, pos), pos };
	return { first, last };
}

template <class BiIt, class UnPred> 
std::pair <BiIt, BiIt> gather(BiIt first, BiIt last, BiIt pos, UnPred pred)
{
	using value_type = typename std::iterator_traits<BiIt>::value_type;
	return { std::stable_partition(first, pos, [&](const value_type& x){ return !pred(x); }), 
			 std::stable_partition(pos, last, pred) };
}

} // namespace helper

} // namespace panda

#endif // HELPER_ALGORITHM_H
