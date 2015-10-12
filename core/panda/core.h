#ifndef PANDA_CORE_H
#define PANDA_CORE_H

#include <panda/helper/system/config.h>

#ifdef PANDA_BUILD_CORE
#	define PANDA_CORE_API EXPORT_DYNAMIC_LIBRARY
#else
#	define PANDA_CORE_API IMPORT_DYNAMIC_LIBRARY
#endif

#endif
