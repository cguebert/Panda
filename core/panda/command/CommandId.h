#ifndef COMMANDID_H
#define COMMANDID_H

#include <panda/core.h>
#include <typeinfo>

namespace panda
{

PANDA_CORE_API int getCommandId(const std::type_info& type);

template <class T>
int getCommandId() { return getCommandId(typeid(T)); }

} // namespace panda

#endif
