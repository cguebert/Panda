#ifndef PANDA_CORE_H
#define PANDA_CORE_H

#include <panda/helper/system/config.h>

#ifdef PANDA_BUILD_CORE
#	define PANDA_CORE_API SOFA_EXPORT_DYNAMIC_LIBRARY
#else
#	define PANDA_CORE_API SOFA_IMPORT_DYNAMIC_LIBRARY
#endif

#endif
