#ifndef DATATRAITS_H
#define DATATRAITS_H

#include <panda/Data.h>
#include <panda/Animation.h>

#include <QVariant>
#include <QRectF>
#include <QPointF>
#include <QVector>
#include <QStringList>
#include <QTextStream>

namespace panda
{

//***************************************************************//

template<class T>
class data_trait
{
public:
	typedef Data<T> data_type;
	typedef T value_type;

	enum { is_single = 1 };
	enum { is_vector = 0 };
	enum { is_animation = 0 };

	static int valueType() { return BaseData::getValueTypeOf<value_type>(); }
	static size_t size(const data_type& /*d*/) { return 1; }
	static void clear(data_type& d, size_t /*size*/, bool init)
	{
		if(init)
			d.setValue(T());
	}
	static QVariant getBaseValue(const data_type& d, size_t /*index*/)
	{
		QVariant temp;
		temp.setValue(d.getValue());
		return temp;
	}
	static void fromBaseValue(data_type& d, QVariant val, size_t /*index*/)
	{
		d.setValue(val.value<T>());
	}
	static QTextStream& writeValue(QTextStream& stream, const value_type& v)
	{
		return stream << v;
	}
	static QTextStream& readValue(QTextStream& stream, value_type& v)
	{
		return stream >> v;
	}
	static void copyValue(data_type* data, const BaseData* parent)
	{
		// First we try without conversion
		if(parent->isVector())
		{
			// The parent is a vector of T
			const Data< QVector<T> >* castedVectorParent = dynamic_cast<const Data< QVector<T> >*>(parent);
			if(castedVectorParent)
			{
				if(castedVectorParent->getValue().size())
					data->setValue(castedVectorParent->getValue()[0]);
				else
					data->setValue(T());
				return;
			}
		}
		else if(parent->isSingleValue())
		{
			// Same type
			const Data<T>* castedParent = dynamic_cast<const Data<T>*>(parent);
			if(castedParent)
			{
				data->setValue(castedParent->getValue());
				return;
			}
		}

		// Else we use QVariant for a conversion
		data->setValue(parent->getBaseValue(0).value<T>());
	}
};

template<class T>
class data_trait< QVector<T> >
{
public:
	typedef QVector<T> vector_type;
	typedef Data<vector_type> data_type;
	typedef T value_type;
	typedef data_trait<T> base_traits;

	enum { is_single = 0 };
	enum { is_vector = 1 };
	enum { is_animation = 0 };

	static int valueType() { return BaseData::getValueTypeOf<value_type>(); }
	static size_t size(const data_type& d) { return d.getValue().size(); }
	static void clear(data_type& d, size_t size, bool init)
	{
		vector_type& v = *d.beginEdit();
		if(init)
			v.clear();
		v.resize(size);
		d.endEdit();
	}
	static QVariant getBaseValue(const data_type& d, size_t index)
	{
		QVariant temp;
		if(index < 0 || index >= size(d))
			temp.setValue(T());
		else
			temp.setValue(d.getValue()[index]);
		return temp;
	}
	static void fromBaseValue(data_type& d, QVariant val, size_t index)
	{
		vector_type& vec = *d.beginEdit();
		if(vec.size() <= static_cast<int>(index))
			vec.resize(index+1);
		vec[index] = val.value<T>();
		d.endEdit();
	}
	static QTextStream& writeValue(QTextStream& stream, const vector_type& vec)
	{
		int size = vec.size();
		if(size)
		{
			base_traits::writeValue(stream, vec[0]);
			for(int i=1; i<size; ++i)
			{
				stream << " ";
				base_traits::writeValue(stream, vec[i]);
			}
		}
		return stream;
	}
	static QTextStream& readValue(QTextStream& stream, vector_type& vec)
	{
		vec.clear();
		T t = T();
		while(!stream.atEnd())
		{
			base_traits::readValue(stream, t);
			vec.append(t);
		}
		return stream;
	}
	static void copyValue(data_type* data, const BaseData* parent)
	{
		// First we try without conversion
		if(parent->isVector())
		{
			// Same type (both vectors)
			const Data< QVector<T> >* castedParent = dynamic_cast<const Data< QVector<T> >*>(parent);
			if(castedParent)
			{
				data->setValue(castedParent->getValue());
				return;
			}
		}
		else if(parent->isAnimation())
		{
			// The parent is not a vector of T, but an animation of type T
			const Data< Animation<T> >* castedAnimationParent = dynamic_cast<const Data< Animation<T> >*>(parent);
			if(castedAnimationParent)
			{
				QVector<T>& vec = *data->beginEdit();
				vec = castedAnimationParent->getValue().getValues().toVector();
				data->endEdit();
				return;
			}
		}
		else if(parent->isSingleValue())
		{
			// The parent is not a vector of T, but a single value of type T
			const Data<T>* castedSingleValueParent = dynamic_cast<const Data<T>*>(parent);
			if(castedSingleValueParent)
			{
				QVector<T>& vec = *data->beginEdit();
				vec.clear();
				vec.append(castedSingleValueParent->getValue());
				data->endEdit();
				return;
			}
		}

		// Else we use QVariant for a conversion
		QVector<T>& value = *data->beginEdit();
		value.clear();
		int size = parent->getSize();
		for(int i=0; i<size; ++i)
			value.append(parent->getBaseValue(i).value<T>());
		data->endEdit();
	}
};

template<class T>
class data_trait< Animation<T> >
{
public:
	typedef Animation<T> animation_type;
	typedef Data<animation_type> data_type;
	typedef T value_type;
	typedef data_trait<T> base_traits;

	enum { is_single = 0 };
	enum { is_vector = 0 };
	enum { is_animation = 1 };

	static int valueType() { return BaseData::getValueTypeOf<value_type>(); }
	static size_t size(const data_type& d) { return d.getValue().size(); }
	static void clear(data_type& d, size_t /*size*/, bool /*init*/)
	{
		d.beginEdit()->clear();
		d.endEdit();
	}
	static QVariant getBaseValue(const data_type& d, size_t index)
	{
		QVariant temp;
		if(index < 0 || index >= size(d))
			temp.setValue(T());
		else
			temp.setValue(d.getValue().getValueAtIndexConst(index));
		return temp;
	}
	static void fromBaseValue(data_type& d, QVariant val, size_t index)
	{
		animation_type& anim = *d.beginEdit();
		anim.getValueAtIndex(index) = val.value<T>();
		d.endEdit();
	}
	static QTextStream& writeValue(QTextStream& stream, const animation_type& anim)
	{
		int size = anim.size();
		if(size)
		{
			typename animation_type::Iterator iter = anim.getIterator();
			while(iter.hasNext())
			{
				iter.next();
				stream << iter.key() << " ";
				base_traits::writeValue(stream, iter.value());
				stream << " ";
			}
		}
		return stream;
	}
	static QTextStream& readValue(QTextStream& stream, animation_type& anim)
	{
		anim.clear();
		T val = T();
		double key;
		while(!stream.atEnd())
		{
			stream >> key;
			base_traits::readValue(stream, val);
			anim.add(key, val);
		}
		return stream;
	}
	static void copyValue(data_type* data, const BaseData* parent)
	{
		// Without conversion
		if(parent->isAnimation())
		{
			// Same type (both animations)
			const Data< Animation<T> >* castedAnimationParent = dynamic_cast<const Data< Animation<T> >*>(parent);
			if(castedAnimationParent)
			{
				data->setValue(castedAnimationParent->getValue());
				return;
			}
		}

		// Not accepting conversions from non-animation datas
		Animation<T>& value = *data->beginEdit();
		value.clear();
		data->endEdit();
	}
};

//***************************************************************//
// Overrides for writeValue

template<>
QTextStream& data_trait<QColor>::writeValue(QTextStream& stream, const QColor& v)
{ return stream << QString("#%1").arg(v.rgba(), 8, 16, QChar('0')).toUpper(); }

template<>
QTextStream& data_trait<QPointF>::writeValue(QTextStream& stream, const QPointF& v)
{ return stream << v.x() << " " << v.y(); }

template<>
QTextStream& data_trait<QRectF>::writeValue(QTextStream& stream, const QRectF& v)
{ return stream << v.left() << " " << v.top() << " " << v.right() << " " << v.bottom(); }

template<>
QTextStream& data_trait<QImage>::writeValue(QTextStream& stream, const QImage&)
{ return stream; } // Use a SaveImage object instead

//***************************************************************//
// Overrides for readValue

template<>
QTextStream& data_trait<QColor>::readValue(QTextStream& stream, QColor& v)
{
	QString temp;
	stream >> temp;
	if(temp.startsWith('#'))
		temp = temp.mid(1);
	v.setRgba(temp.toUInt(nullptr, 16));
	return stream;
}

template<>
QTextStream& data_trait<QRectF>::readValue(QTextStream& stream, QRectF& v)
{
	double l, t, r, b;
	stream >> l >> t >> r >> b;
	v = QRectF(l, t, r-l, b-t).normalized();
	return stream;
}

template<>
QTextStream& data_trait<QPointF>::readValue(QTextStream& stream, QPointF& v)
{ return stream >> v.rx() >> v.ry(); }

template<>
QTextStream& data_trait< QVector<QString> >::readValue(QTextStream& stream, QVector<QString>& v)
{
	v.clear();
	while(!stream.atEnd())
	{
		QString t = stream.readLine();
		if(!t.isEmpty())
			v.append(t);
	}
	return stream;
}

template<>
QTextStream& data_trait<QImage>::readValue(QTextStream& stream, QImage&)
{ return stream; } // Not saving images (save it as a separate file and use a LoadImage object)

template<>
QTextStream& data_trait<QString>::readValue(QTextStream& stream, QString& v)
{ v = stream.readLine(); return stream; }

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

//***************************************************************//

template<class T>
T valueFromString(const QString& text)
{
	T val();
	QTextStream stream(&text, QIODevice::ReadOnly);
	data_trait<T>::readValue(stream, val);
	return val;
}

template<class T>
QString valueToString(const T& val)
{
	QString tempString;
	QTextStream stream(&tempString, QIODevice::WriteOnly);
	data_trait<T>::writeValue(stream, val);
	return tempString;
}

} // namespace panda

#endif // DATATRAITS_H
