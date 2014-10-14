#ifndef COMMANDID_H
#define COMMANDID_H

#include <panda/core.h>
#include <typeinfo>

template<class T> class RegisterWidget;

PANDA_CORE_API int getCommandId(const std::type_info& type);

template <class T>
int getCommandId() { return getCommandId(typeid(T)); }

#endif
