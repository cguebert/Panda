#ifndef HELPER_GL_H
#define HELPER_GL_H

#include <QtGui/qopengl.h>

#ifdef PANDA_DOUBLE
#define GL_PREAL GL_DOUBLE
#else
#define GL_PREAL GL_FLOAT
#endif

#endif // HELPER_GL_H
