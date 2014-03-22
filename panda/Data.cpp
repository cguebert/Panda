#include <panda/helper/system/Config.h>
#include <panda/DataFactory.h>
#include <panda/Data.inl>
#include <panda/types/TypeConverter.h>

#include <QColor>
#include <QString>

#include <QVector>

namespace panda
{

template class Data<int>;
template class Data<PReal>;
template class Data<QColor>;
template class Data<QString>;

template class Data< QVector<int> >;
template class Data< QVector<PReal> >;
template class Data< QVector<QColor> >;
template class Data< QVector<QString> >;

int intDataClass = RegisterData< int >();
int doubleDataClass = RegisterData< PReal >();
int colorDataClass = RegisterData< QColor >();
int stringDataClass = RegisterData< QString >();

int intVectorDataClass = RegisterData< QVector<int> >();
int doubleVectorDataClass = RegisterData< QVector<PReal> >();
int colorVectorDataClass = RegisterData< QVector<QColor> >();
int stringVectorDataClass = RegisterData< QVector<QString> >();

} // namespace panda

void convertType(const int& from, PReal& to)
{ to = static_cast<PReal>(from); }
void convertType(const PReal& from, int& to)
{ to = static_cast<int>(from); }

panda::types::RegisterTypeConverter<int, PReal> intRealConverter;
panda::types::RegisterTypeConverter<PReal, int> realIntConverter;
