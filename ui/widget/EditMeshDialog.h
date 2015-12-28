#ifndef EDITMESHDIALOG_H
#define EDITMESHDIALOG_H

#include <panda/types/Mesh.h>

#include <ui/widget/DataWidget.h>

#include <QDialog>

class EditMeshDialog : public QDialog
{
	Q_OBJECT
public:
	explicit EditMeshDialog(BaseDataWidget* parent, bool readOnly, QString name);

	void readFromData(const panda::types::Mesh& v);
	void writeToData(panda::types::Mesh& v);

protected:
	panda::types::Mesh m_mesh;

	typedef std::shared_ptr<BaseDataWidget> DataWidgetPtr;
	DataWidgetPtr m_pointsWidget, m_edgesWidget, m_trianglesWidget;
};

#endif // EDITMESHDIALOG_H
