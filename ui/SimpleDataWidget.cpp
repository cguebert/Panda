#include <ui/SimpleDataWidget.h>
#include <helper/Factory.h>

Creator<DataWidgetFactory, SimpleDataWidget<int> > DWClass_int("default",true);
Creator<DataWidgetFactory, SimpleDataWidget<int, checkbox_data_widget_trait> > DWClass_checkbox("checkbox",true);
Creator<DataWidgetFactory, SimpleDataWidget<double> > DWClass_double("default",true);
Creator<DataWidgetFactory, SimpleDataWidget<QString> > DWClass_string("default",true);

Creator<DataWidgetFactory, SimpleDataWidget<QColor> > DWClass_color("default",true);
Creator<DataWidgetFactory, SimpleDataWidget<QPointF> > DWClass_point("default",true);
Creator<DataWidgetFactory, SimpleDataWidget<QRectF> > DWClass_rect("default",true);
