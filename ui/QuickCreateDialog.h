#ifndef QUICKCREATEDIALOG_H
#define QUICKCREATEDIALOG_H

#include <QDialog>

namespace panda
{
	class PandaDocument;
}

class QLineEdit;
class QListWidget;

class QuickCreateDialog : public QDialog
{
    Q_OBJECT
public:
	explicit QuickCreateDialog(panda::PandaDocument* doc, QWidget *parent = 0);

protected:
	panda::PandaDocument* document;
	QLineEdit* lineEdit;
	QListWidget* listWidget;
	QStringList menuStringsList;

public slots:
	void searchTextChanged();
	void createObject();
};

#endif // QUICKCREATEDIALOG_H
