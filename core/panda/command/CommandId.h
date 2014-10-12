#ifndef COMMANDID_H
#define COMMANDID_H

#include <typeinfo>

template<class T> class RegisterWidget;

int getCommandId(const std::type_info& type);

template <class T>
int getCommandId() { return getCommandId(typeid(T)); }

#endif
