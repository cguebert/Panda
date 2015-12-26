#include <panda/core.h>
#include <panda/Data.h>

#include <QString>
#include <vector>

namespace panda
{

namespace types
{

#ifndef PANDA_BUILD_CORE
extern template class PANDA_CORE_API Data<int>;
extern template class PANDA_CORE_API Data<PReal>;
extern template class PANDA_CORE_API Data<QString>;

extern template class PANDA_CORE_API Data< std::vector<int> >;
extern template class PANDA_CORE_API Data< std::vector<PReal> >;
extern template class PANDA_CORE_API Data< std::vector<QString> >;
#endif


} // namespace types

} // namespace panda
