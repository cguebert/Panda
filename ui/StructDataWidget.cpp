#include <ui/StructDataWidget.h>
#include <ui/SimpleDataWidget.h>
#include <helper/Factory.h>

Creator<DataWidgetFactory, SimpleDataWidget<QPointF, open_dialog_data_widget_container< QPointF > > > DWClass_point("default",true);
Creator<DataWidgetFactory, SimpleDataWidget<QRectF, open_dialog_data_widget_container< QRectF > > > DWClass_rect("default",true);

Creator<DataWidgetFactory, SimpleDataWidget<QVector<int>, open_dialog_data_widget_container< QVector<int> > > > DWClass_ints_list("default",true);
Creator<DataWidgetFactory, SimpleDataWidget<QVector<double>, open_dialog_data_widget_container< QVector<double> > > > DWClass_doubles_list("default",true);
Creator<DataWidgetFactory, SimpleDataWidget<QVector<QPointF>, open_dialog_data_widget_container< QVector<QPointF> > > > DWClass_points_list("default",true);
Creator<DataWidgetFactory, SimpleDataWidget<QVector<QRectF>, open_dialog_data_widget_container< QVector<QRectF> > > > DWClass_rects_list("default",true);
Creator<DataWidgetFactory, SimpleDataWidget<QVector<QString>, open_dialog_data_widget_container< QVector<QString> > > > DWClass_strings_list("default",true);
