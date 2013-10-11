#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>

#include <panda/types/Animation.h>
#include <panda/types/Gradient.h>
#include <QPointF>
#include <QColor>
#include <QRectF>
#include <QString>
#include <QVector>

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
	}
};

template <class T>
class GeneratorUser : public PandaObject
{
public:
	PANDA_CLASS(PANDA_TEMPLATE(GeneratorUser, T), PandaObject)

	GeneratorUser(PandaDocument *doc)
		: PandaObject(doc)
		, userValue("value", "The value you want to store", this)
	{
		addOutput(&userValue);
		userValue.setReadOnly(false);
	}

protected:
	CustomData<T> userValue;
};

int GeneratorUser_IntegerClass = RegisterObject< GeneratorUser<int> >("Generator/Integer/Integer user value").setName("Integer value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_DoubleClass = RegisterObject< GeneratorUser<double> >("Generator/Real/Real user value").setName("Real value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_PointClass = RegisterObject< GeneratorUser<QPointF> >("Generator/Point/Point user value").setName("Point value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_ColorClass = RegisterObject< GeneratorUser<QColor> >("Generator/Color/Color user value").setName("Color value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_RectClass = RegisterObject< GeneratorUser<QRectF> >("Generator/Rectangle/Rectangle user value").setName("Rectangle value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_StringClass = RegisterObject< GeneratorUser<QString> >("Generator/Text/Text user value").setName("Text value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_GradientClass = RegisterObject< GeneratorUser<types::Gradient> >("Generator/Gradient/Gradient user value").setName("Gradient value").setDescription("Lets you store a value for use in other objects");

int GeneratorUser_VectorIntegerClass = RegisterObject< GeneratorUser< QVector<int> > >("Generator/Integer/Integers list user value").setName("Integers list").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_VectorDoubleClass = RegisterObject< GeneratorUser< QVector<double> > >("Generator/Real/Reals list user value").setName("Reals list").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_VectorPointClass = RegisterObject< GeneratorUser< QVector<QPointF> > >("Generator/Point/Points list user value").setName("Points list").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_VectorColorClass = RegisterObject< GeneratorUser< QVector<QColor> > >("Generator/Color/Colors list user value").setName("Colors list").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_VectorRectClass = RegisterObject< GeneratorUser< QVector<QRectF> > >("Generator/Rectangle/Rectangles list user value").setName("Rectangles list").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_VectorStringClass = RegisterObject< GeneratorUser< QVector<QString> > >("Generator/Text/Texts list user value").setName("Texts list").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_VectorGradientClass = RegisterObject< GeneratorUser< QVector<types::Gradient> > >("Generator/Gradient/Gradients list user value").setName("Gradients list").setDescription("Lets you store a value for use in other objects");

int GeneratorUser_AnimationDoubleClass = RegisterObject< GeneratorUser< types::Animation<double> > >("Generator/Real/Reals animation user value").setName("Reals animation").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_AnimationPointClass = RegisterObject< GeneratorUser< types::Animation<QPointF> > >("Generator/Point/Points animation user value").setName("Points animation").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_AnimationColorClass = RegisterObject< GeneratorUser< types::Animation<QColor> > >("Generator/Color/Colors animation user value").setName("Colors animation").setDescription("Lets you store a value for use in other objects");

} // namespace Panda
