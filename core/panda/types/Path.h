#ifndef TYPES_PATH_H
#define TYPES_PATH_H

#include <panda/core.h>
#include <panda/types/Point.h>

#include <QVector>

namespace panda
{

namespace types
{

class PANDA_CORE_API Path : public QVector<Point>
{
public:
	Path& operator=(const QVector<Point>& v);
};

#ifndef PANDA_BUILD_CORE
extern template class PANDA_CORE_API Data< Path >;
extern template class PANDA_CORE_API Data< QVector<Path> >;
#endif

} // namespace types

} // namespace panda

#endif // TYPES_PATH_H
