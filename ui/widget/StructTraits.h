#ifndef STRUCTTRAITS_H
#define STRUCTTRAITS_H

#include <QVector>
#include <QStringList>

template<class T>
class FlatDataTrait
{
public:
	typedef T value_type;
	typedef T item_type;

	static int size() { return 1; }
	static QStringList header() { return QStringList(); }
	static const item_type get(const value_type& d, int /*i*/= 0) { return d; }
	static void set(value_type& d, const item_type& v, int /*i*/= 0) { d = v; }
	static QString toString(const value_type& /*d*/) { return QString(); }
};

template<>
static QString FlatDataTrait<QString>::toString(const value_type& d)
{ return d; }

//***************************************************************//

template<class T>
class VectorDataTrait
{
public:
	typedef T vector_type;
	typedef T row_type;

	enum { is_vector = 0 };
	enum { is_single = 1 };
	static int size(const vector_type&) { return 1; }
	static const row_type* get(const vector_type& v, int i=0)
	{
		return !i ? &v : nullptr;
	}
	static row_type* get(vector_type& v, int i=0)
	{
		return !i ? &v : nullptr;
	}
	static void set(vector_type& v, const row_type& r, int i=0)
	{
		if(!i)
			v = r;
	}
	static void resize(vector_type& /*v*/, int /*s*/) {}
};

//***************************************************************//

template<class T>
class VectorDataTrait< QVector<T> >
{
public:
	typedef QVector<T> vector_type;
	typedef T row_type;

	enum { is_vector = 1 };
	enum { is_single = 0 };
	static int size(const vector_type& v) { return v.size(); }
	static QStringList header(const vector_type&) { return QStringList; }
	static const row_type* get(const vector_type& v, int i=0)
	{
		return (i<size(v)) ? &(v[i]) : nullptr;
	}
	static row_type* get(vector_type& v, int i=0)
	{
		return (i<size(v)) ? &(v[i]) : nullptr;
	}
	static void set(vector_type& v, const row_type& r, int i=0)
	{
		if(i < size(v))
			v[i] = r;
	}
	static void resize(vector_type& v, int s)
	{
		v.resize(s);
	}
};

#endif // STRUCTTRAITS_H
