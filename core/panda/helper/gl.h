#ifndef HELPER_GL_H
#define HELPER_GL_H

#ifdef PANDA_DOUBLE
#define GL_PREAL GL_DOUBLE
#define glTranslateReal glTranslated
#define glRotateReal glRotated
#define glScaleReal glScaled
#define glVertex2r glVertex2d
#else
#define GL_PREAL GL_FLOAT
#define glTranslateReal glTranslatef
#define glRotateReal glRotatef
#define glScaleReal glScalef
#define glVertex2r glVertex2f
#endif

#endif // HELPER_GL_H
