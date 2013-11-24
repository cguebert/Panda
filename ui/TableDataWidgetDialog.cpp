#include <ui/TableDataWidgetDialog.h>
#include <ui/SimpleDataWidget.h>
#include <ui/DataWidgetFactory.h>

RegisterWidget<OpenDialogDataWidget<QPointF, TableDataWidgetDialog<QPointF> > > DWClass_point("default");
RegisterWidget<OpenDialogDataWidget<QRectF, TableDataWidgetDialog<QRectF> > > DWClass_rect("default");

RegisterWidget<OpenDialogDataWidget<QVector<int>, TableDataWidgetDialog<QVector<int> > > > DWClass_ints_list("default");
RegisterWidget<OpenDialogDataWidget<QVector<double>, TableDataWidgetDialog<QVector<double> > > > DWClass_doubles_list("default");
RegisterWidget<OpenDialogDataWidget<QVector<QPointF>, TableDataWidgetDialog<QVector<QPointF> > > > DWClass_points_list("default");
RegisterWidget<OpenDialogDataWidget<QVector<QRectF>, TableDataWidgetDialog<QVector<QRectF> > > > DWClass_rects_list("default");
RegisterWidget<OpenDialogDataWidget<QVector<QString>, TableDataWidgetDialog<QVector<QString> > > > DWClass_strings_list("default");
