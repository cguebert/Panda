#include <panda/ObjectFactory.h>

#include <panda/types/Animation.h>
#include <panda/types/Color.h>
#include <panda/types/Gradient.h>
#include <panda/types/Mesh.h>
#include <panda/types/Path.h>
#include <panda/types/Rect.h>
#include <panda/types/Shader.h>

#include "LoadValue.h"

#include <QVector>

namespace panda {

int LoadValue_ColorClass = RegisterObject< LoadValue<types::Color> >("File/Color/Load color").setDescription("Load a value from a file");
int LoadValue_DoubleClass = RegisterObject< LoadValue<PReal> >("File/Real/Load real").setDescription("Load a value from a file");
int LoadValue_GradientClass = RegisterObject< LoadValue<types::Gradient> >("File/Gradient/Load gradient").setDescription("Load a value from a file");
int LoadValue_IntegerClass = RegisterObject< LoadValue<int> >("File/Integer/Load integer").setDescription("Load a value from a file");
int LoadValue_MeshClass = RegisterObject< LoadValue<types::Mesh> >("File/Mesh/Load mesh").setDescription("Load a value from a file");
int LoadValue_PathClass = RegisterObject< LoadValue<types::Path> >("File/Path/Load path").setDescription("Load a value from a file");
int LoadValue_PointClass = RegisterObject< LoadValue<types::Point> >("File/Point/Load point").setDescription("Load a value from a file");
int LoadValue_RectClass = RegisterObject< LoadValue<types::Rect> >("File/Rectangle/Load rectangle").setDescription("Load a value from a file");
int LoadValue_ShaderClass = RegisterObject< LoadValue<types::Shader> >("File/Shader/Load shader").setDescription("Load a value from a file");
int LoadValue_StringClass = RegisterObject< LoadValue<QString> >("File/Text/Load text").setDescription("Load a value from a file");

int LoadValue_VectorColorClass = RegisterObject< LoadValue< QVector<types::Color> > >("File/Color/Load colors list").setDescription("Load a value from a file");
int LoadValue_VectorDoubleClass = RegisterObject< LoadValue< QVector<PReal> > >("File/Real/Load reals list").setDescription("Load a value from a file");
int LoadValue_VectorGradientClass = RegisterObject< LoadValue< QVector<types::Gradient> > >("File/Gradient/Load gradients list").setDescription("Load a value from a file");
int LoadValue_VectorIntegerClass = RegisterObject< LoadValue< QVector<int> > >("File/Integer/Load integers list").setDescription("Load a value from a file");
int LoadValue_VectorMeshClass = RegisterObject< LoadValue< QVector<types::Mesh> > >("File/Mesh/Load meshes list").setDescription("Load a value from a file");
int LoadValue_VectorPathClass = RegisterObject< LoadValue< QVector<types::Path> > >("File/Path/Load paths list").setDescription("Load a value from a file");
int LoadValue_VectorPointClass = RegisterObject< LoadValue< QVector<types::Point> > >("File/Point/Load points list").setDescription("Load a value from a file");
int LoadValue_VectorRectClass = RegisterObject< LoadValue< QVector<types::Rect> > >("File/Rectangle/Load rectangles list").setDescription("Load a value from a file");
int LoadValue_VectorShaderClass = RegisterObject< LoadValue< QVector<types::Shader> > >("File/Shader/Load shaders list").setDescription("Load a value from a file");
int LoadValue_VectorStringClass = RegisterObject< LoadValue< QVector<QString> > >("File/Text/Load texts list").setDescription("Load a value from a file");

int LoadValue_AnimationColorClass = RegisterObject< LoadValue< types::Animation<types::Color> > >("File/Color/Load colors animation").setDescription("Load a value from a file");
int LoadValue_AnimationDoubleClass = RegisterObject< LoadValue< types::Animation<PReal> > >("File/Real/Load reals animation").setDescription("Load a value from a file");
int LoadValue_AnimationGradientClass = RegisterObject< LoadValue< types::Animation<types::Gradient> > >("File/Gradient/Load gradients animation").setDescription("Load a value from a file");
int LoadValue_AnimationPointClass = RegisterObject< LoadValue< types::Animation<types::Point> > >("File/Point/Load points animation").setDescription("Load a value from a file");

} // namespace Panda
