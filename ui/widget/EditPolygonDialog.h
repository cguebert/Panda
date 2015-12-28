#ifndef EDITPOLYGONDIALOG_H
#define EDITPOLYGONDIALOG_H

#include <panda/types/Polygon.h>

#include <ui/widget/DataWidget.h>

#include <QDialog>

class EditPolygonDialog : public QDialog
{
	Q_OBJECT
public:
	explicit EditPolygonDialog(BaseDataWidget* parent, bool readOnly, QString name);

	void readFromData(const panda::types::Polygon& v);
	void writeToData(panda::types::Polygon& v);

protected:
	panda::types::Polygon m_polygon;

	typedef std::shared_ptr<BaseDataWidget> DataWidgetPtr;
	DataWidgetPtr m_contourWidget, m_holesWidget;
};

#endif // EDITPOLYGONDIALOG_H
