#include <ui/widget/AnimationDataWidgetDialog.h>
#include <ui/widget/OpenDialogDataWidget.h>
#include <ui/widget/DataWidgetFactory.h>

using panda::types::Animation;

RegisterWidget<OpenDialogDataWidget<Animation<double>, AnimationDataWidgetDialog<Animation<double> > > > DWClass_reals_animation("default");
RegisterWidget<OpenDialogDataWidget<Animation<QPointF>, AnimationDataWidgetDialog<Animation<QPointF> > > > DWClass_points_animation("default");
RegisterWidget<OpenDialogDataWidget<Animation<QColor>, AnimationDataWidgetDialog<Animation<QColor> > > > DWClass_colors_animation("default");
