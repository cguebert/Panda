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
	auto last = std::remove(container.begin(),container.end(), value);
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

} // namespace helper

} // namespace panda

#endif // HELPER_ALGORITHM_H
