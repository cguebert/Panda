#include <panda/core.h>
#include <panda/Data.h>

#include <string>
#include <vector>

namespace panda
{

namespace types
{

#ifndef PANDA_BUILD_CORE
extern template class PANDA_CORE_API Data<int>;
extern template class PANDA_CORE_API Data<PReal>;
extern template class PANDA_CORE_API Data<std::string>;

extern template class PANDA_CORE_API Data< std::vector<int> >;
extern template class PANDA_CORE_API Data< std::vector<PReal> >;
extern template class PANDA_CORE_API Data< std::vector<std::string> >;
#endif


} // namespace types

} // namespace panda
