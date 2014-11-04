#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Group.h>

#include <panda/types/Animation.h>
#include <panda/types/Color.h>
#include <panda/types/Gradient.h>
#include <panda/types/Mesh.h>
#include <panda/types/Path.h>
#include <panda/types/Rect.h>
#include <panda/types/Shader.h>

#include <QString>
#include <QVector>

#include <QTimer>
#include <QFile>
#include <QDomDocument>

namespace panda {

template <class T>
class CustomData : public Data<T>
{
public:
	CustomData(const QString& name, const QString& help, PandaObject* owner)
		: Data<T>(name, help, owner)
	{ }
	virtual void save(QDomDocument& doc, QDomElement& elem)
	{
		Data<T>::save(doc, elem);
		QString w = getWidget();
		if(w != "default")
			elem.setAttribute("widget", w);
		QString d = getWidgetData();
		if(!d.isEmpty())
			elem.setAttribute("widgetData", d);
	}
};

template <class T>
class GeneratorUser : public BaseGeneratorUser
{
public:
	PANDA_CLASS(PANDA_TEMPLATE(GeneratorUser, T), BaseGeneratorUser)

	GeneratorUser(PandaDocument *doc)
		: BaseGeneratorUser(doc)
		, userValue("input", "The value you want to store", this)
		, output(initData(&output, "value", "The value stored"))
	{
		addInput(&userValue);
		addOutput(&output);

		output.setDisplayed(false);

		dataSetParent(&output, &userValue);
	}

	bool hasConnectedInput()
	{
		return userValue.getParent();
	}

	bool hasConnectedOutput()
	{
		return !output.getOutputs().empty();
	}

	BaseData* getInputUserData()
	{
		return &userValue;
	}

	BaseData* getOutputUserData()
	{
		return &output;
	}

protected:
	CustomData<T> userValue;
	Data<T> output;
};

int GeneratorUser_ColorClass = RegisterObject< GeneratorUser<types::Color> >("Generator/Color/Color user value").setName("Color value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_DoubleClass = RegisterObject< GeneratorUser<PReal> >("Generator/Real/Real user value").setName("Real value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_GradientClass = RegisterObject< GeneratorUser<types::Gradient> >("Generator/Gradient/Gradient user value").setName("Gradient value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_IntegerClass = RegisterObject< GeneratorUser<int> >("Generator/Integer/Integer user value").setName("Integer value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_MeshClass = RegisterObject< GeneratorUser<types::Mesh> >("Generator/Mesh/Mesh user value").setName("Mesh value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_PathClass = RegisterObject< GeneratorUser<types::Path> >("Generator/Path/Path user value").setName("Path value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_PointClass = RegisterObject< GeneratorUser<types::Point> >("Generator/Point/Point user value").setName("Point value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_RectClass = RegisterObject< GeneratorUser<types::Rect> >("Generator/Rectangle/Rectangle user value").setName("Rectangle value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_ShaderClass = RegisterObject< GeneratorUser<types::Shader> >("Generator/Shader/Shader user value").setName("Shader value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_StringClass = RegisterObject< GeneratorUser<QString> >("Generator/Text/Text user value").setName("Text value").setDescription("Lets you store a value for use in other objects");

int GeneratorUser_VectorColorClass = RegisterObject< GeneratorUser< QVector<types::Color> > >("Generator/Color/Colors list user value").setName("Colors list").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_VectorDoubleClass = RegisterObject< GeneratorUser< QVector<PReal> > >("Generator/Real/Reals list user value").setName("Reals list").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_VectorGradientClass = RegisterObject< GeneratorUser< QVector<types::Gradient> > >("Generator/Gradient/Gradients list user value").setName("Gradients list").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_VectorIntegerClass = RegisterObject< GeneratorUser< QVector<int> > >("Generator/Integer/Integers list user value").setName("Integers list").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_VectorMeshClass = RegisterObject< GeneratorUser< QVector<types::Mesh> > >("Generator/Mesh/Meshes list user value").setName("Meshes list").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_VectorPathClass = RegisterObject< GeneratorUser< QVector<types::Path> > >("Generator/Path/Paths list user value").setName("Paths list").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_VectorPointClass = RegisterObject< GeneratorUser< QVector<types::Point> > >("Generator/Point/Points list user value").setName("Points list").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_VectorRectClass = RegisterObject< GeneratorUser< QVector<types::Rect> > >("Generator/Rectangle/Rectangles list user value").setName("Rectangles list").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_VectorShaderClass = RegisterObject< GeneratorUser< QVector<types::Shader> > >("Generator/Shader/Shaders list user value").setName("Shaders list").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_VectorStringClass = RegisterObject< GeneratorUser< QVector<QString> > >("Generator/Text/Texts list user value").setName("Texts list").setDescription("Lets you store a value for use in other objects");

int GeneratorUser_AnimationColorClass = RegisterObject< GeneratorUser< types::Animation<types::Color> > >("Generator/Color/Colors animation user value").setName("Colors animation").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_AnimationDoubleClass = RegisterObject< GeneratorUser< types::Animation<PReal> > >("Generator/Real/Reals animation user value").setName("Reals animation").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_AnimationGradientClass = RegisterObject< GeneratorUser< types::Animation<types::Gradient> > >("Generator/Gradient/Gradients animation user value").setName("Gradients animation").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_AnimationPointClass = RegisterObject< GeneratorUser< types::Animation<types::Point> > >("Generator/Point/Points animation user value").setName("Points animation").setDescription("Lets you store a value for use in other objects");

} // namespace Panda

panda::ModuleHandle userValueModule = REGISTER_MODULE
		.setDescription("Object to store values for the UI or to & from files.")
		.setLicense("GPL")
		.setVersion("1.0");
