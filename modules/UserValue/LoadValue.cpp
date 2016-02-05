#include <panda/object/ObjectFactory.h>

#include <panda/types/Animation.h>
#include <panda/types/Color.h>
#include <panda/types/Gradient.h>
#include <panda/types/Mesh.h>
#include <panda/types/Path.h>
#include <panda/types/Rect.h>
#include <panda/types/Shader.h>

#include "LoadValue.h"

namespace panda {

int LoadValue_ColorClass = RegisterObject< LoadValue<types::Color> >("File/Color/Load color").setDescription("Load a value from a file");
int LoadValue_DoubleClass = RegisterObject< LoadValue<float> >("File/Real/Load real").setDescription("Load a value from a file");
int LoadValue_GradientClass = RegisterObject< LoadValue<types::Gradient> >("File/Gradient/Load gradient").setDescription("Load a value from a file");
int LoadValue_IntegerClass = RegisterObject< LoadValue<int> >("File/Integer/Load integer").setDescription("Load a value from a file");
int LoadValue_MeshClass = RegisterObject< LoadValue<types::Mesh> >("File/Mesh/Load mesh").setDescription("Load a value from a file");
int LoadValue_PathClass = RegisterObject< LoadValue<types::Path> >("File/Path/Load path").setDescription("Load a value from a file");
int LoadValue_PointClass = RegisterObject< LoadValue<types::Point> >("File/Point/Load point").setDescription("Load a value from a file");
int LoadValue_RectClass = RegisterObject< LoadValue<types::Rect> >("File/Rectangle/Load rectangle").setDescription("Load a value from a file");
int LoadValue_ShaderClass = RegisterObject< LoadValue<types::Shader> >("File/Shader/Load shader").setDescription("Load a value from a file");
int LoadValue_StringClass = RegisterObject< LoadValue<std::string> >("File/Text/Load text").setDescription("Load a value from a file");

int LoadValue_VectorColorClass = RegisterObject< LoadValue< std::vector<types::Color> > >("File/Color/Load colors list").setDescription("Load a value from a file");
int LoadValue_VectorDoubleClass = RegisterObject< LoadValue< std::vector<float> > >("File/Real/Load reals list").setDescription("Load a value from a file");
int LoadValue_VectorGradientClass = RegisterObject< LoadValue< std::vector<types::Gradient> > >("File/Gradient/Load gradients list").setDescription("Load a value from a file");
int LoadValue_VectorIntegerClass = RegisterObject< LoadValue< std::vector<int> > >("File/Integer/Load integers list").setDescription("Load a value from a file");
int LoadValue_VectorMeshClass = RegisterObject< LoadValue< std::vector<types::Mesh> > >("File/Mesh/Load meshes list").setDescription("Load a value from a file");
int LoadValue_VectorPathClass = RegisterObject< LoadValue< std::vector<types::Path> > >("File/Path/Load paths list").setDescription("Load a value from a file");
int LoadValue_VectorPointClass = RegisterObject< LoadValue< std::vector<types::Point> > >("File/Point/Load points list").setDescription("Load a value from a file");
int LoadValue_VectorRectClass = RegisterObject< LoadValue< std::vector<types::Rect> > >("File/Rectangle/Load rectangles list").setDescription("Load a value from a file");
int LoadValue_VectorShaderClass = RegisterObject< LoadValue< std::vector<types::Shader> > >("File/Shader/Load shaders list").setDescription("Load a value from a file");
int LoadValue_VectorStringClass = RegisterObject< LoadValue< std::vector<std::string> > >("File/Text/Load texts list").setDescription("Load a value from a file");

int LoadValue_AnimationColorClass = RegisterObject< LoadValue< types::Animation<types::Color> > >("File/Color/Load colors animation").setDescription("Load a value from a file");
int LoadValue_AnimationDoubleClass = RegisterObject< LoadValue< types::Animation<float> > >("File/Real/Load reals animation").setDescription("Load a value from a file");
int LoadValue_AnimationGradientClass = RegisterObject< LoadValue< types::Animation<types::Gradient> > >("File/Gradient/Load gradients animation").setDescription("Load a value from a file");
int LoadValue_AnimationPointClass = RegisterObject< LoadValue< types::Animation<types::Point> > >("File/Point/Load points animation").setDescription("Load a value from a file");

} // namespace Panda
