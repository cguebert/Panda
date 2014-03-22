#include <ui/widget/AnimationDataWidgetDialog.h>
#include <ui/widget/OpenDialogDataWidget.h>
#include <ui/widget/DataWidgetFactory.h>
#include <panda/types/Point.h>

using panda::types::Point;
using panda::types::Animation;

RegisterWidget<OpenDialogDataWidget<Animation<PReal>, AnimationDataWidgetDialog<Animation<PReal> > > > DWClass_reals_animation("default");
RegisterWidget<OpenDialogDataWidget<Animation<Point>, AnimationDataWidgetDialog<Animation<Point> > > > DWClass_points_animation("default");
RegisterWidget<OpenDialogDataWidget<Animation<QColor>, AnimationDataWidgetDialog<Animation<QColor> > > > DWClass_colors_animation("default");
