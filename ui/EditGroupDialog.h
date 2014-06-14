#ifndef EDITGROUPDIALOG_H
#define EDITGROUPDIALOG_H

#include <QDialog>

namespace panda
{
	class PandaDocument;
	class Group;
	class BaseData;
}

class QLineEdit;
class QTableWidget;
class QTableWidgetItem;

class EditGroupDialog : public QDialog
{
	Q_OBJECT
public:
	explicit EditGroupDialog(panda::Group* group, QWidget* parent = nullptr);

	void populateRow(int rowIndex, panda::BaseData* data);

protected:
	panda::Group* m_group;
	QTableWidget* m_tableWidget;
	QLineEdit *m_editGroupName, *m_editDataName, *m_editDataHelp;
	panda::BaseData* m_selectedData;
	int m_selectedRow;

public slots:
	void itemClicked(QTableWidgetItem*);
	void moveUp();
	void moveDown();
	void updateGroup();
	void dataNameEdited(QString);
	void dataHelpEdited(QString);
};

#endif // EDITGROUP
