#ifndef EDITGROUPDIALOG_H
#define EDITGROUPDIALOG_H

#include <QDialog>

#include <map>
#include <vector>

namespace panda
{
	class PandaDocument;
	class Group;
	class BaseData;
}

class QLabel;
class QLineEdit;
class QListWidget;
class QListWidgetItem;

class EditGroupDialog : public QDialog
{
	Q_OBJECT
public:
	explicit EditGroupDialog(panda::Group* group, QWidget* parent = nullptr);

protected:
	using Datas = std::vector<panda::BaseData*>;
	using DataTextMap = std::map<panda::BaseData*, QString>;

	void fillList(QListWidget* listWidget, const Datas& datas);

	panda::Group* m_group;
	QListWidget *m_inputsListWidget, *m_outputsListWidget;
	QLabel *m_dataTypeLabel;
	QLineEdit *m_editGroupName, *m_editDataName, *m_editDataHelp;
	panda::BaseData* m_selectedData = nullptr;
	QListWidgetItem* m_selection = nullptr;
	DataTextMap m_datasName, m_datasDescription;

public slots:
	void itemClicked(QListWidgetItem*);
	void moveUp();
	void moveDown();
	void updateGroup();
	void dataNameEdited(QString);
	void dataHelpEdited(QString);
};

#endif // EDITGROUP
