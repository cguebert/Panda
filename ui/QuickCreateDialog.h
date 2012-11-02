#ifndef DATASEDITDIALOG_H
#define DATASEDITDIALOG_H

#include <QDialog>
#include <QAbstractListModel>

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

#endif // DATASEDITDIALOG_H
