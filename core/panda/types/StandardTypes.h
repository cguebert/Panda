#include <panda/core.h>
#include <panda/Data.h>

#include <QString>
#include <QVector>

namespace panda
{

namespace types
{

#ifndef PANDA_BUILD_CORE
extern template class PANDA_CORE_API Data<int>;
extern template class PANDA_CORE_API Data<PReal>;
extern template class PANDA_CORE_API Data<QString>;

extern template class PANDA_CORE_API Data< QVector<int> >;
extern template class PANDA_CORE_API Data< QVector<PReal> >;
extern template class PANDA_CORE_API Data< QVector<QString> >;
#endif


} // namespace types

} // namespace panda
