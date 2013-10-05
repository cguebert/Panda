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

int intDataClass = RegisterData< int >();
int doubleDataClass = RegisterData< double >();
int colorDataClass = RegisterData< QColor >();
int pointDataClass = RegisterData< QPointF >();
int rectDataClass = RegisterData< QRectF >();
int stringDataClass = RegisterData< QString >();
int imageDataClass = RegisterData< QImage >();

int intVectorDataClass = RegisterData< QVector<int> >();
int doubleVectorDataClass = RegisterData< QVector<double> >();
int colorVectorDataClass = RegisterData< QVector<QColor> >();
int pointVectorDataClass = RegisterData< QVector<QPointF> >();
int rectVectorDataClass = RegisterData< QVector<QRectF> >();
int stringVectorDataClass = RegisterData< QVector<QString> >();
int imageVectorDataClass = RegisterData< QVector<QImage> >();

int doubleAnimationDataClass = RegisterData< Animation<double> >();
int colorAnimationDataClass = RegisterData< Animation<QColor> >();
int pointAnimationDataClass = RegisterData< Animation<QPointF> >();

} // namespace panda
