#include <ui/SimpleDataWidget.h>
#include <helper/Factory.h>

Creator<DataWidgetFactory, SimpleDataWidget<int> > DWClass_int("default",true);
// Creator<DataWidgetFactory, checkbox_data_widget_trait > DWClass_bool("checkbox",true);
Creator<DataWidgetFactory, SimpleDataWidget<double> > DWClass_double("default",true);
Creator<DataWidgetFactory, SimpleDataWidget<QString> > DWClass_string("default",true);


