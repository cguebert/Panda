#include <panda/object/ObjectFactory.h>

#include <panda/types/Animation.h>
#include <panda/types/Color.h>
#include <panda/types/Gradient.h>
#include <panda/types/Mesh.h>
#include <panda/types/Polygon.h>
#include <panda/types/Rect.h>
#include <panda/types/Shader.h>

#include "UserValue.h"

namespace panda {

int GeneratorUser_ColorClass = RegisterObject< GeneratorUser<types::Color> >("Generator/Color/Color user value").setName("Color value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_DoubleClass = RegisterObject< GeneratorUser<float> >("Generator/Real/Real user value").setName("Real value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_GradientClass = RegisterObject< GeneratorUser<types::Gradient> >("Generator/Gradient/Gradient user value").setName("Gradient value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_IntegerClass = RegisterObject< GeneratorUser<int> >("Generator/Integer/Integer user value").setName("Integer value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_MeshClass = RegisterObject< GeneratorUser<types::Mesh> >("Generator/Mesh/Mesh user value").setName("Mesh value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_PathClass = RegisterObject< GeneratorUser<types::Path> >("Generator/Path/Path user value").setName("Path value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_PointClass = RegisterObject< GeneratorUser<types::Point> >("Generator/Point/Point user value").setName("Point value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_PolygonClass = RegisterObject< GeneratorUser<types::Polygon> >("Generator/Polygon/Polygon user value").setName("Polygon value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_RectClass = RegisterObject< GeneratorUser<types::Rect> >("Generator/Rectangle/Rectangle user value").setName("Rectangle value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_ShaderClass = RegisterObject< GeneratorUser<types::Shader> >("Generator/Shader/Shader user value").setName("Shader value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_StringClass = RegisterObject< GeneratorUser<std::string> >("Generator/Text/Text user value").setName("Text value").setDescription("Lets you store a value for use in other objects");

int GeneratorUser_VectorColorClass = RegisterObject< GeneratorUser< std::vector<types::Color> > >("Generator/Color/Colors list user value").setName("Colors list").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_VectorDoubleClass = RegisterObject< GeneratorUser< std::vector<float> > >("Generator/Real/Reals list user value").setName("Reals list").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_VectorGradientClass = RegisterObject< GeneratorUser< std::vector<types::Gradient> > >("Generator/Gradient/Gradients list user value").setName("Gradients list").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_VectorIntegerClass = RegisterObject< GeneratorUser< std::vector<int> > >("Generator/Integer/Integers list user value").setName("Integers list").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_VectorMeshClass = RegisterObject< GeneratorUser< std::vector<types::Mesh> > >("Generator/Mesh/Meshes list user value").setName("Meshes list").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_VectorPathClass = RegisterObject< GeneratorUser< std::vector<types::Path> > >("Generator/Path/Paths list user value").setName("Paths list").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_VectorPointClass = RegisterObject< GeneratorUser< std::vector<types::Point> > >("Generator/Point/Points list user value").setName("Points list").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_VectorPolygonlass = RegisterObject< GeneratorUser< std::vector<types::Polygon> > >("Generator/Polygon/Polygons list user value").setName("Polygons list").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_VectorRectClass = RegisterObject< GeneratorUser< std::vector<types::Rect> > >("Generator/Rectangle/Rectangles list user value").setName("Rectangles list").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_VectorShaderClass = RegisterObject< GeneratorUser< std::vector<types::Shader> > >("Generator/Shader/Shaders list user value").setName("Shaders list").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_VectorStringClass = RegisterObject< GeneratorUser< std::vector<std::string> > >("Generator/Text/Texts list user value").setName("Texts list").setDescription("Lets you store a value for use in other objects");

int GeneratorUser_AnimationColorClass = RegisterObject< GeneratorUser< types::Animation<types::Color> > >("Generator/Color/Colors animation user value").setName("Colors animation").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_AnimationDoubleClass = RegisterObject< GeneratorUser< types::Animation<float> > >("Generator/Real/Reals animation user value").setName("Reals animation").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_AnimationGradientClass = RegisterObject< GeneratorUser< types::Animation<types::Gradient> > >("Generator/Gradient/Gradients animation user value").setName("Gradients animation").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_AnimationPointClass = RegisterObject< GeneratorUser< types::Animation<types::Point> > >("Generator/Point/Points animation user value").setName("Points animation").setDescription("Lets you store a value for use in other objects");

} // namespace Panda

panda::ModuleHandle userValueModule = REGISTER_MODULE
		.setDescription("Object to store values for the UI or to & from files.")
		.setLicense("GPL")
		.setVersion("1.0");
