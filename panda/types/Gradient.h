#ifndef GRADIENT_H
#define GRADIENT_H

#include <QVector>
#include <QColor>

namespace panda
{

namespace types
{

class Gradient
{
public:
	typedef QPair<qreal, QColor> GradientStop;
	typedef QVector<GradientStop> GradientStops;

	enum Extend { EXTEND_PAD = 0, EXTEND_REPEAT = 1, EXTEND_REFLECT = 2 };

	Gradient();

	void clear();

	void add(qreal position, QColor color);
	QColor get(qreal position) const;

	void setExtend(int method);
	int getExtend() const;

	void setStops(GradientStops stopsPoints);
	GradientStops getStops() const;

	inline bool operator==(const Gradient& rhs)
	{
		return extend == rhs.extend && stops == rhs.stops;
	}

	inline bool operator!=(const Gradient& rhs)
	{
		return !(*this == rhs);
	}

protected:
	qreal extendPos(qreal position) const;

	static qreal interpolate(qreal v1, qreal v2, qreal amt);
	static QColor interpolate(const GradientStop& s1, const GradientStop& s2, qreal pos);
	static QColor interpolate(const QColor& v1, const QColor& v2, qreal amt);

	GradientStops stops;
	Extend extend;
};

} // namespace types

} // namespace panda

#endif // GRADIENT_H
