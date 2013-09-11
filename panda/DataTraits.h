#ifndef DATATRAITS_H
#define DATATRAITS_H

#include <panda/BaseData.h>
#include <panda/Animation.h>

#include <QRectF>
#include <QPointF>
#include <QVector>
#include <QStringList>

namespace panda
{

//***************************************************************//

template<class T>
class data_trait
{
public:
	typedef T value_type;

	enum { is_single = 1 };
	enum { is_vector = 0 };
	enum { is_animation = 0 };

	static int getValueType() { return BaseData::getValueTypeOf<value_type>(); }
};

template<class T>
class data_trait< QVector<T> >
{
public:
	typedef T value_type;

	enum { is_single = 0 };
	enum { is_vector = 1 };
	enum { is_animation = 0 };

	static int getValueType() { return BaseData::getValueTypeOf<value_type>(); }
};

template<class T>
class data_trait< Animation<T> >
{
public:
	typedef T value_type;

	enum { is_single = 0 };
	enum { is_vector = 0 };
	enum { is_animation = 1 };

	static int getValueType() { return BaseData::getValueTypeOf<value_type>(); }
};

//***************************************************************//

template<class T>
class flat_data_trait
{
public:
	typedef T value_type;
	typedef T item_type;

	static size_t size() { return 1; }
	static QStringList header() { return QStringList(); }
	static const item_type get(const value_type& d, size_t /*i*/= 0) { return d; }
	static void set(value_type& d, const item_type& v, size_t /*i*/= 0) { d = v; }
};

//***************************************************************//

template<>
class flat_data_trait<QPointF>
{
public:
	typedef QPointF value_type;
	typedef qreal item_type;

	static size_t size() { return 2; }
	static QStringList header()
	{
		QStringList header;
		header << "X" << "Y";
		return header;
	}
	static const item_type get(const value_type& d, size_t i = 0)
	{
		switch(i)
		{
		case 0: return d.x();
		case 1: return d.y();
		}

		return 0.;
	}
	static void set(value_type& d, const item_type& v, size_t i = 0)
	{
		switch(i)
		{
		case 0: return d.setX(v);
		case 1: return d.setY(v);
		}
	}
};

//***************************************************************//

template<>
class flat_data_trait<QRectF>
{
public:
	typedef QRectF value_type;
	typedef qreal item_type;

	static size_t size() { return 4; }
	static QStringList header()
	{
		QStringList header;
		header << "Left" << "Top" << "Right" << "Bottom";
		return header;
	}
	static const item_type get(const value_type& d, size_t i = 0)
	{
		switch(i)
		{
		case 0: return d.left();
		case 1: return d.top();
		case 2: return d.right();
		case 3: return d.bottom();
		}

		return 0.;
	}
	static void set(value_type& d, const item_type& v, size_t i = 0)
	{
		switch(i)
		{
		case 0: return d.setLeft(v);
		case 1: return d.setTop(v);
		case 2: return d.setRight(v);
		case 3: return d.setBottom(v);
		}
	}
};

//***************************************************************//

template<class T>
class vector_data_trait
{
public:
	typedef T vector_type;
	typedef T row_type;

	enum { is_vector = 0 };
	enum { is_single = 1 };
	static size_t size(const vector_type&) { return 1; }
	static const row_type* get(const vector_type& v, size_t i=0)
	{
		return !i ? &v : nullptr;
	}
	static void set(vector_type& v, const row_type& r, size_t i=0)
	{
		if(!i)
			v = r;
	}
	static void resize(vector_type& /*v*/, size_t /*s*/) {}
};

//***************************************************************//

template<class T>
class vector_data_trait< QVector<T> >
{
public:
	typedef QVector<T> vector_type;
	typedef T row_type;

	enum { is_vector = 1 };
	enum { is_single = 0 };
	static size_t size(const vector_type& v) { return v.size(); }
	static QStringList header(const vector_type&) { return QStringList{}; }
	static const row_type* get(const vector_type& v, size_t i=0)
	{
		return (i<size(v)) ? &(v[i]) : nullptr;
	}
	static void set(vector_type& v, const row_type& r, size_t i=0)
	{
		if(i < size(v))
			v[i] = r;
	}
	static void resize(vector_type& v, size_t s)
	{
		v.resize(s);
	}
};

} // namespace panda

#endif // DATATRAITS_H
