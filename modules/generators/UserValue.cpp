#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Animation.h>

#include <QPointF>
#include <QColor>
#include <QRectF>
#include <QString>
#include <QVector>

namespace panda {

template <class T>
class GeneratorUser : public PandaObject
{
public:
    GeneratorUser(PandaDocument *doc)
        : PandaObject(doc)
        , userValue(initData(&userValue, T(), "value", "The value you want to store"))
    {
        addOutput(&userValue);
        userValue.setReadOnly(false);
    }

protected:
    Data<T> userValue;
};

int GeneratorUser_IntegerClass = RegisterObject("Generator/Integer/Integer user value").setClass< GeneratorUser<int> >().setName("Integer value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_DoubleClass = RegisterObject("Generator/Real/Real user value").setClass< GeneratorUser<double> >().setName("Real value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_PointClass = RegisterObject("Generator/Point/Point user value").setClass< GeneratorUser<QPointF> >().setName("Point value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_ColorClass = RegisterObject("Generator/Color/Color user value").setClass< GeneratorUser<QColor> >().setName("Color value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_RectClass = RegisterObject("Generator/Rectangle/Rectangle user value").setClass< GeneratorUser<QRectF> >().setName("Rectangle value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_StringClass = RegisterObject("Generator/Text/Text user value").setClass< GeneratorUser<QString> >().setName("Text value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_GradientClass = RegisterObject("Generator/Gradient/Gradient user value").setClass< GeneratorUser<Gradient> >().setName("Gradient value").setDescription("Lets you store a value for use in other objects");

int GeneratorUser_VectorIntegerClass = RegisterObject("Generator/Integer/Integers list user value").setClass< GeneratorUser< QVector<int> > >().setName("Integers list").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_VectorDoubleClass = RegisterObject("Generator/Real/Reals list user value").setClass< GeneratorUser< QVector<double> > >().setName("Reals list").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_VectorPointClass = RegisterObject("Generator/Point/Points list user value").setClass< GeneratorUser< QVector<QPointF> > >().setName("Points list").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_VectorColorClass = RegisterObject("Generator/Color/Colors list user value").setClass< GeneratorUser< QVector<QColor> > >().setName("Colors list").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_VectorRectClass = RegisterObject("Generator/Rectangle/Rectangles list user value").setClass< GeneratorUser< QVector<QRectF> > >().setName("Rectangles list").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_VectorStringClass = RegisterObject("Generator/Text/Texts list user value").setClass< GeneratorUser< QVector<QString> > >().setName("Texts list").setDescription("Lets you store a value for use in other objects");

int GeneratorUser_AnimationDoubleClass = RegisterObject("Generator/Real/Reals animation user value").setClass< GeneratorUser< Animation<double> > >().setName("Reals animation").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_AnimationPointClass = RegisterObject("Generator/Point/Points animation user value").setClass< GeneratorUser< Animation<QPointF> > >().setName("Points animation").setDescription("Lets you store a value for use in other objects");

} // namespace Panda
