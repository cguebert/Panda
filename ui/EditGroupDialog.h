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
	explicit EditGroupDialog(panda::Group* group, QWidget *parent = 0);

	void populateTable();
	void populateRow(int rowIndex, panda::BaseData* data);

protected:
	panda::Group* group;
	QTableWidget* tableWidget;
	QLineEdit* editGroupName;
	QLineEdit* editDataName;
	QLineEdit* editDataHelp;
	panda::BaseData* selectedData;
	int selectedRow;

public slots:
	void itemClicked(QTableWidgetItem*);
	void moveUp();
	void moveDown();
	void updateGroup();
	void dataNameEdited(QString);
	void dataHelpEdited(QString);
};

#endif // EDITGROUP
