#ifndef HELPER_ACCESSOR_H
#define HELPER_ACCESSOR_H

#include <panda/Data.h>

namespace panda
{

template<class T>
class DataAccessor
{
public:
	typedef T container_type;
	typedef T value_type;
	typedef value_type& reference;
	typedef value_type* pointer;
	typedef const value_type& const_reference;
	typedef const value_type* const_pointer;

protected:
	container_type& vref;

public:
	explicit DataAccessor(container_type& container) : vref(container) {}
	~DataAccessor() {}

	const_reference ref() const { return vref; }
	reference wref() { return vref; }

	operator  const_reference () const { return  vref; }
	const_pointer   operator->() const { return &vref; }
	const_reference operator* () const { return  vref; }

	operator  reference () { return  vref; }
	pointer   operator->() { return &vref; }
	reference operator* () { return  vref; }

	template<class U> void operator=(const U& v) { vref = v; }
};

//***************************************************************//

template<class T>
class DataAccessorVector
{
public:
	typedef T container_type;
	typedef typename container_type::size_type size_type;
	typedef typename container_type::value_type value_type;
	typedef typename container_type::reference reference;
	typedef typename container_type::const_reference const_reference;
	typedef typename container_type::iterator iterator;
	typedef typename container_type::const_iterator const_iterator;

protected:
	container_type& vref;

public:
	DataAccessorVector(container_type& container) : vref(container) {}
	~DataAccessorVector() {}

	const container_type& ref() const { return vref; }
	container_type& wref() { return vref; }

	bool empty() const { return vref.empty(); }
	size_type size() const { return vref.size(); }

	const_reference operator[](size_type i) const { return vref[i]; }
	reference operator[](size_type i) { return vref[i]; }

	const_iterator begin() const { return vref.begin(); }
	iterator begin() { return vref.begin(); }
	const_iterator end() const { return vref.end(); }
	iterator end() { return vref.end(); }

	void clear() { vref.clear(); }
	void resize(size_type s) { vref.resize(s); }
	void reserve(size_type s) { vref.reserve(s); }
	void push_back(const_reference v) { vref.push_back(v); }

	template<class U> void operator=(const U& v) { vref = v; }
	container_type& operator<<(const container_type& v) { return vref << v; }
	container_type& operator<<(const value_type& v) { return vref << v; }
};

template<class T>
class DataAccessor< QVector<T> > : public DataAccessorVector< QVector<T> >
{
public:
	typedef DataAccessorVector< QVector<T> > Inherit;
	typedef typename Inherit::container_type container_type;
	typedef typename Inherit::value_type value_type;
	DataAccessor(container_type& c) : Inherit(c) {}
	template<class U> void operator=(const U& v) { Inherit::operator=(v); }
	container_type& operator<<(const container_type& v) { return Inherit::operator<<(v); }
	container_type& operator<<(const value_type& v) { return Inherit::operator<<(v); }
};

} // namespace panda

#endif // HELPER_RANDOM_H
