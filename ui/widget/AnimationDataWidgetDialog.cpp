#include <ui/widget/AnimationDataWidgetDialog.h>
#include <ui/widget/DataWidgetFactory.h>
#include <ui/widget/OpenDialogDataWidget.h>

using panda::types::Animation;

RegisterWidget<OpenDialogDataWidget<Animation<PReal>, AnimationDataWidgetDialog<Animation<PReal> > > > DWClass_reals_animation("default");
