#include <panda/Data.h>
#include <panda/DataFactory.h>

#include <QColor>
#include <QPointF>
#include <QRectF>
#include <QString>
#include <QImage>

#include <QVector>
#include <panda/Animation.h>

namespace panda
{

template class Data<int>;
template class Data<double>;
template class Data<QColor>;
template class Data<QPointF>;
template class Data<QRectF>;
template class Data<QString>;
template class Data<QImage>;

template class Data< QVector<int> >;
template class Data< QVector<double> >;
template class Data< QVector<QColor> >;
template class Data< QVector<QPointF> >;
template class Data< QVector<QRectF> >;
template class Data< QVector<QString> >;
template class Data< QVector<QImage> >;

template class Data< Animation<double> >;
template class Data< Animation<QColor> >;
template class Data< Animation<QPointF> >;

int intDataClass = RegisterData< Data<int> >();
int doubleDataClass = RegisterData< Data<double> >();
int colorDataClass = RegisterData< Data<QColor> >();
int pointDataClass = RegisterData< Data<QPointF> >();
int rectDataClass = RegisterData< Data<QRectF> >();
int stringDataClass = RegisterData< Data<QString> >();
int imageDataClass = RegisterData< Data<QImage> >();

int intVectorDataClass = RegisterData< Data< QVector<int> > >();
int doubleVectorDataClass = RegisterData< Data< QVector<double> > >();
int colorVectorDataClass = RegisterData< Data< QVector<QColor> > >();
int pointVectorDataClass = RegisterData< Data< QVector<QPointF> > >();
int rectVectorDataClass = RegisterData< Data< QVector<QRectF> > >();
int stringVectorDataClass = RegisterData< Data< QVector<QString> > >();
int imageVectorDataClass = RegisterData< Data< QVector<QImage> > >();

int doubleAnimationDataClass = RegisterData< Data< Animation<double> > >();
int colorAnimationDataClass = RegisterData< Data< Animation<QColor> > >();
int pointAnimationDataClass = RegisterData< Data< Animation<QPointF> > >();

} // namespace panda
