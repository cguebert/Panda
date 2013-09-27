#include <ui/TableDataWidgetDialog.h>
#include <ui/SimpleDataWidget.h>

Creator<DataWidgetFactory, OpenDialogDataWidget<QPointF, TableDataWidgetDialog<QPointF> > > DWClass_point("default",true);
Creator<DataWidgetFactory, OpenDialogDataWidget<QRectF, TableDataWidgetDialog<QRectF> > > DWClass_rect("default",true);

Creator<DataWidgetFactory, OpenDialogDataWidget<QVector<int>, TableDataWidgetDialog<QVector<int> > > > DWClass_ints_list("default",true);
Creator<DataWidgetFactory, OpenDialogDataWidget<QVector<double>, TableDataWidgetDialog<QVector<double> > > > DWClass_doubles_list("default",true);
Creator<DataWidgetFactory, OpenDialogDataWidget<QVector<QPointF>, TableDataWidgetDialog<QVector<QPointF> > > > DWClass_points_list("default",true);
Creator<DataWidgetFactory, OpenDialogDataWidget<QVector<QRectF>, TableDataWidgetDialog<QVector<QRectF> > > > DWClass_rects_list("default",true);
Creator<DataWidgetFactory, OpenDialogDataWidget<QVector<QString>, TableDataWidgetDialog<QVector<QString> > > > DWClass_strings_list("default",true);
