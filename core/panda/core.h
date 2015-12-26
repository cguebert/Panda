#ifndef PANDA_CORE_H
#define PANDA_CORE_H

#include <panda/helper/system/config.h>

#ifndef WIN32
#	define EXPORT_DYNAMIC_LIBRARY
#	define IMPORT_DYNAMIC_LIBRARY
#else
#	define EXPORT_DYNAMIC_LIBRARY __declspec( dllexport )
#	define IMPORT_DYNAMIC_LIBRARY __declspec( dllimport )
#	pragma warning(disable : 4251)
#endif

#ifdef PANDA_BUILD_CORE
#	define PANDA_CORE_API EXPORT_DYNAMIC_LIBRARY
#else
#	define PANDA_CORE_API IMPORT_DYNAMIC_LIBRARY
#endif

#endif
