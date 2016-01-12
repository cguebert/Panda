#ifndef HELPER_SYSTEM_CONFIG_H
#define HELPER_SYSTEM_CONFIG_H

#ifdef PANDA_DOUBLE
typedef double PReal;
#else
typedef float PReal;
#endif

#ifdef _MSC_VER
#ifndef _USE_MATH_DEFINES
# define _USE_MATH_DEFINES 1 // required to get M_PI from math.h
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

#ifndef WIN32
#	define EXPORT_DYNAMIC_LIBRARY
#   define IMPORT_DYNAMIC_LIBRARY
#else
#	define EXPORT_DYNAMIC_LIBRARY __declspec( dllexport )
#   define IMPORT_DYNAMIC_LIBRARY __declspec( dllimport )
#   ifdef _MSC_VER
#		pragma warning(disable : 4251) /* class 'type' needs to have dll-interface to be used by clients of class 'type2' */
#		pragma warning(disable : 4275) /* non - DLL-interface classkey 'identifier' used as base for DLL-interface classkey 'identifier' */
#   endif
#endif

#endif // HELPER_SYSTEM_CONFIG_H
