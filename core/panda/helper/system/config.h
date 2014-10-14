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
#	define SOFA_EXPORT_DYNAMIC_LIBRARY
#   define SOFA_IMPORT_DYNAMIC_LIBRARY
#else
#	define SOFA_EXPORT_DYNAMIC_LIBRARY __declspec( dllexport )
#   define SOFA_IMPORT_DYNAMIC_LIBRARY __declspec( dllimport )
#   ifdef _MSC_VER
#       pragma warning(disable : 4231)
#       pragma warning(disable : 4910)
#   endif
#endif

#endif // HELPER_SYSTEM_CONFIG_H
