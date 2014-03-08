#include <ui/widget/TableDataWidgetDialog.h>
#include <ui/widget/ListDataWidgetDialog.h>
#include <ui/widget/OpenDialogDataWidget.h>
#include <ui/widget/DataWidgetFactory.h>
#include <panda/types/Path.h>
#include <QStringList>

using panda::types::Path;

template<>
class VectorDataTrait<Path>
{
public:
	typedef Path vector_type;
	typedef QPointF row_type;

	enum { is_vector = 1 };
	enum { is_single = 0 };
	static int size(const vector_type& v) { return v.size(); }
	static QStringList header(const vector_type&) { return QStringList(); }
	static const row_type* get(const vector_type& v, int i=0)
	{
		return (i<size(v)) ? &(v[i]) : nullptr;
	}
	static row_type* get(vector_type& v, int i=0)
	{
		return (i<size(v)) ? &(v[i]) : nullptr;
	}
	static void set(vector_type& v, const row_type& r, int i=0)
	{
		if(i < size(v))
			v[i] = r;
	}
	static void resize(vector_type& v, int s)
	{
		v.resize(s);
	}
};

RegisterWidget<OpenDialogDataWidget<Path, TableDataWidgetDialog<Path> > > DWClass_strings_list("default");
RegisterWidget<OpenDialogDataWidget<QVector<Path>, ListDataWidgetDialog<QVector<Path> > > > DWClass_paths_list_generic("generic");
