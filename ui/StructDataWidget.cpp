#include <ui/StructDataWidget.h>
#include <ui/SimpleDataWidget.h>
#include <helper/Factory.h>

Creator<DataWidgetFactory, SimpleDataWidget<QPointF, open_dialog_data_widget_container< QPointF > > > DWClass_point("default",true);
Creator<DataWidgetFactory, SimpleDataWidget<QRectF, open_dialog_data_widget_container< QRectF > > > DWClass_rect("default",true);

Creator<DataWidgetFactory, SimpleDataWidget<QVector<QPointF>, open_dialog_data_widget_container< QVector<QPointF> > > > DWClass_points_list("default",true);
