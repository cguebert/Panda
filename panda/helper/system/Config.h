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

#endif // HELPER_SYSTEM_CONFIG_H
