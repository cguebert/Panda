#ifndef HELPER_ACCESSOR_H
#define HELPER_ACCESSOR_H

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
	container_type& m_vref;

public:
	explicit DataAccessor(container_type& container) : m_vref(container) {}
	~DataAccessor() {}

	const_reference ref() const { return m_vref; }
	reference wref() { return m_vref; }

	operator  const_reference () const { return  m_vref; }
	const_pointer   operator->() const { return &m_vref; }
	const_reference operator* () const { return  m_vref; }

	operator  reference () { return  m_vref; }
	pointer   operator->() { return &m_vref; }
	reference operator* () { return  m_vref; }

	template<class U> void operator=(const U& v) { m_vref = v; }
};

//****************************************************************************//

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
	container_type& m_vref;

public:
	DataAccessorVector(container_type& container) : m_vref(container) {}
	~DataAccessorVector() {}

	const container_type& ref() const { return m_vref; }
	container_type& wref() { return m_vref; }

	bool empty() const { return m_vref.empty(); }
	size_type size() const { return m_vref.size(); }

	const_reference operator[](size_type i) const { return m_vref[i]; }
	reference operator[](size_type i) { return m_vref[i]; }

	const_iterator begin() const { return m_vref.begin(); }
	iterator begin() { return m_vref.begin(); }
	const_iterator end() const { return m_vref.end(); }
	iterator end() { return m_vref.end(); }

	void clear() { m_vref.clear(); }
	void resize(size_type s) { m_vref.resize(s); }
	void reserve(size_type s) { m_vref.reserve(s); }
	void push_back(const_reference v) { m_vref.push_back(v); }

	template<class U> void operator=(const U& v) { m_vref = v; }
	container_type& operator<<(const container_type& v) { return m_vref << v; }
	container_type& operator<<(const value_type& v) { return m_vref << v; }
};

template<class T>
class DataAccessor< std::vector<T> > : public DataAccessorVector< std::vector<T> >
{
public:
	typedef DataAccessorVector< std::vector<T> > Inherit;
	typedef typename Inherit::container_type container_type;
	typedef typename Inherit::value_type value_type;
	DataAccessor(container_type& c) : Inherit(c) {}
	template<class U> void operator=(const U& v) { Inherit::operator=(v); }
	container_type& operator<<(const container_type& v) { return Inherit::operator<<(v); }
	container_type& operator<<(const value_type& v) { return Inherit::operator<<(v); }
};

} // namespace panda

#endif // HELPER_RANDOM_H
