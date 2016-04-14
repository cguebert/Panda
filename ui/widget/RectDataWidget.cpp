#include <ui/widget/DataWidgetFactory.h>
#include <ui/widget/ListDataWidgetDialog.h>
#include <ui/widget/OpenDialogDataWidget.h>
#include <ui/widget/TableDataWidgetDialog.h>

#include <panda/types/Rect.h>

using panda::types::Rect;

template<>
class FlatDataTrait<Rect>
{
public:
	typedef Rect value_type;
	typedef float item_type;

	static int size() { return 4; }
	static QStringList header()
	{
		QStringList header;
		header << "Left" << "Top" << "Right" << "Bottom";
		return header;
	}
	static const item_type get(const value_type& d, int i = 0)
	{
		switch(i)
		{
		case 0: return d.left();
		case 1: return d.top();
		case 2: return d.right();
		case 3: return d.bottom();
		}

		return 0.;
	}
	static void set(value_type& d, const item_type& v, int i = 0)
	{
		switch(i)
		{
		case 0: d.setLeft(v); break;
		case 1: d.setTop(v); break;
		case 2: d.setRight(v); break;
		case 3: d.setBottom(v); break;
		}
	}
	static QString toString(const value_type& d)
	{
		return QString("%1 %2 %3 %4")
				.arg(d.left()).arg(d.top())
				.arg(d.right()).arg(d.bottom());
	}
};

RegisterWidget<OpenDialogDataWidget<Rect, TableDataWidgetDialog<Rect> > > DWClass_rect("default");
RegisterWidget<OpenDialogDataWidget<std::vector<Rect>, TableDataWidgetDialog<std::vector<Rect> > > > DWClass_rects_list("default");
RegisterWidget<OpenDialogDataWidget<std::vector<Rect>, ListDataWidgetDialog<std::vector<Rect> > > > DWClass_rects_list_generic("generic");
