#include <ui/widget/ListDataWidgetDialog.h>
#include <ui/widget/OpenDialogDataWidget.h>
#include <ui/widget/DataWidgetFactory.h>

RegisterWidget<OpenDialogDataWidget<QVector<int>, ListDataWidgetDialog<QVector<int> > > > DWClass_ints_list_generic("generic");
RegisterWidget<OpenDialogDataWidget<QVector<double>, ListDataWidgetDialog<QVector<double> > > > DWClass_doubles_list_generic("generic");
RegisterWidget<OpenDialogDataWidget<QVector<QColor>, ListDataWidgetDialog<QVector<QColor> > > > DWClass_colors_list_generic("generic");
RegisterWidget<OpenDialogDataWidget<QVector<QPointF>, ListDataWidgetDialog<QVector<QPointF> > > > DWClass_points_list_generic("generic");
RegisterWidget<OpenDialogDataWidget<QVector<QRectF>, ListDataWidgetDialog<QVector<QRectF> > > > DWClass_rects_list_generic("generic");
RegisterWidget<OpenDialogDataWidget<QVector<QString>, ListDataWidgetDialog<QVector<QString> > > > DWClass_strings_list_generic("generic");
