#include <ui/widget/TableDataWidgetDialog.h>
#include <ui/widget/OpenDialogDataWidget.h>
#include <ui/widget/DataWidgetFactory.h>

RegisterWidget<OpenDialogDataWidget<std::vector<int>, TableDataWidgetDialog<std::vector<int> > > > DWClass_ints_list("default");
RegisterWidget<OpenDialogDataWidget<std::vector<PReal>, TableDataWidgetDialog<std::vector<PReal> > > > DWClass_doubles_list("default");
//RegisterWidget<OpenDialogDataWidget<std::vector<QString>, TableDataWidgetDialog<std::vector<QString> > > > DWClass_strings_list("default");
