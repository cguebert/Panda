#include <panda/ObjectFactory.h>

#include <panda/types/Animation.h>
#include <panda/types/Color.h>
#include <panda/types/Gradient.h>
#include <panda/types/Mesh.h>
#include <panda/types/Path.h>
#include <panda/types/Rect.h>
#include <panda/types/Shader.h>

#include "StoreValue.h"

namespace panda {

int StoreValue_ColorClass = RegisterObject< StoreValue<types::Color> >("File/Color/Save color").setDescription("Save a value in a file for later use");
int StoreValue_DoubleClass = RegisterObject< StoreValue<PReal> >("File/Real/Save real").setDescription("Save a value in a file for later use");
int StoreValue_GradientClass = RegisterObject< StoreValue<types::Gradient> >("File/Gradient/Save gradient").setDescription("Save a value in a file for later use");
int StoreValue_IntegerClass = RegisterObject< StoreValue<int> >("File/Integer/Save integer").setDescription("Save a value in a file for later use");
int StoreValue_MeshClass = RegisterObject< StoreValue<types::Mesh> >("File/Mesh/Save mesh").setDescription("Save a value in a file for later use");
int StoreValue_PathClass = RegisterObject< StoreValue<types::Path> >("File/Path/Save path").setDescription("Save a value in a file for later use");
int StoreValue_PointClass = RegisterObject< StoreValue<types::Point> >("File/Point/Save point").setDescription("Save a value in a file for later use");
int StoreValue_RectClass = RegisterObject< StoreValue<types::Rect> >("File/Rectangle/Save rectangle").setDescription("Save a value in a file for later use");
int StoreValue_ShaderClass = RegisterObject< StoreValue<types::Shader> >("File/Shader/Save shader").setDescription("Save a value in a file for later use");
int StoreValue_StringClass = RegisterObject< StoreValue<std::string> >("File/Text/Save text").setDescription("Save a value in a file for later use");

int StoreValue_VectorColorClass = RegisterObject< StoreValue< std::vector<types::Color> > >("File/Color/Save colors list").setDescription("Save a value in a file for later use");
int StoreValue_VectorDoubleClass = RegisterObject< StoreValue< std::vector<PReal> > >("File/Real/Save reals list").setDescription("Save a value in a file for later use");
int StoreValue_VectorGradientClass = RegisterObject< StoreValue< std::vector<types::Gradient> > >("File/Gradient/Save gradients list").setDescription("Save a value in a file for later use");
int StoreValue_VectorIntegerClass = RegisterObject< StoreValue< std::vector<int> > >("File/Integer/Save integers list").setDescription("Save a value in a file for later use");
int StoreValue_VectorMeshClass = RegisterObject< StoreValue< std::vector<types::Mesh> > >("File/Mesh/Save meshes list").setDescription("Save a value in a file for later use");
int StoreValue_VectorPathClass = RegisterObject< StoreValue< std::vector<types::Path> > >("File/Path/Save paths list").setDescription("Save a value in a file for later use");
int StoreValue_VectorPointClass = RegisterObject< StoreValue< std::vector<types::Point> > >("File/Point/Save points list").setDescription("Save a value in a file for later use");
int StoreValue_VectorRectClass = RegisterObject< StoreValue< std::vector<types::Rect> > >("File/Rectangle/Save rectangles list").setDescription("Save a value in a file for later use");
int StoreValue_VectorShaderClass = RegisterObject< StoreValue< std::vector<types::Shader> > >("File/Shader/Save shaders list").setDescription("Save a value in a file for later use");
int StoreValue_VectorStringClass = RegisterObject< StoreValue< std::vector<std::string> > >("File/Text/Save texts list").setDescription("Save a value in a file for later use");

int StoreValue_AnimationColorClass = RegisterObject< StoreValue< types::Animation<types::Color> > >("File/Color/Save colors animation").setDescription("Save a value in a file for later use");
int StoreValue_AnimationDoubleClass = RegisterObject< StoreValue< types::Animation<PReal> > >("File/Real/Save reals animation").setDescription("Save a value in a file for later use");
int StoreValue_AnimationGradientClass = RegisterObject< StoreValue< types::Animation<types::Gradient> > >("File/Gradient/Save gradients animation").setDescription("Save a value in a file for later use");
int StoreValue_AnimationPointClass = RegisterObject< StoreValue< types::Animation<types::Point> > >("File/Point/Save points animation").setDescription("Save a value in a file for later use");


} // namespace Panda
