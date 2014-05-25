#ifndef QUICKCREATEDIALOG_H
#define QUICKCREATEDIALOG_H

#include <QDialog>

namespace panda
{
	class PandaDocument;
}

class GraphView;

class QLineEdit;
class QListWidget;
class QLabel;

class QuickCreateDialog : public QDialog
{
	Q_OBJECT
public:
	explicit QuickCreateDialog(panda::PandaDocument* doc, GraphView* view);

protected:
	panda::PandaDocument* m_document;
	GraphView* m_view;
	QLineEdit* m_lineEdit;
	QLabel* m_descLabel;
	QListWidget* m_listWidget;
	QStringList m_menuStringsList;

public slots:
	void updateDescLabel();
	void searchTextChanged();
	void createObject();
};

#endif // QUICKCREATEDIALOG_H
