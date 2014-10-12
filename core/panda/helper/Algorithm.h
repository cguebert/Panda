#ifndef HELPER_ALGORITHM_H
#define HELPER_ALGORITHM_H

namespace panda
{

namespace helper
{

template<class Container, class Indices>
void removeIndices(Container& values, const Indices& indices)
{
	typedef typename Container::iterator iterator;
	iterator first = std::begin(values);
	iterator last = std::end(values);

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

	values.erase(result, last);
}

} // namespace helper

} // namespace panda

#endif // HELPER_ALGORITHM_H
