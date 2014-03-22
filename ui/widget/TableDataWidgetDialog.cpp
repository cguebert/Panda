#include <ui/widget/TableDataWidgetDialog.h>
#include <ui/widget/OpenDialogDataWidget.h>
#include <ui/widget/DataWidgetFactory.h>

RegisterWidget<OpenDialogDataWidget<QVector<int>, TableDataWidgetDialog<QVector<int> > > > DWClass_ints_list("default");
RegisterWidget<OpenDialogDataWidget<QVector<PReal>, TableDataWidgetDialog<QVector<PReal> > > > DWClass_doubles_list("default");
//RegisterWidget<OpenDialogDataWidget<QVector<QString>, TableDataWidgetDialog<QVector<QString> > > > DWClass_strings_list("default");
