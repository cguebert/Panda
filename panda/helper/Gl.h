#ifndef HELPER_GL_H
#define HELPER_GL_H

#include <QtGui/qopengl.h>

#ifdef PANDA_DOUBLE
#define GL_PREAL GL_DOUBLE
#define glTranslateReal glTranslated
#define glRotateReal glRotated
#define glScaleReal glScaled
#else
#define GL_PREAL GL_FLOAT
#define glTranslateReal glTranslatef
#define glRotateReal glRotatef
#define glScaleReal glScalef
#endif

#endif // HELPER_GL_H
