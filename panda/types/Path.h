#ifndef TYPES_PATH_H
#define TYPES_PATH_H

#include <QVector>
#include <panda/types/Point.h>

namespace panda
{

namespace types
{

class Path : public QVector<Point>
{
public:
	Path& operator=(const QVector<Point>& v);
};

} // namespace types

} // namespace panda

#endif // TYPES_PATH_H
