#ifndef DATASEDITDIALOG_H
#define DATASEDITDIALOG_H

#include <QDialog>

namespace panda
{
	class PandaDocument;
}

class QLineEdit;

class QuickCreateDialog : public QDialog
{
    Q_OBJECT
public:
	explicit QuickCreateDialog(panda::PandaDocument* doc, QWidget *parent = 0);

protected:
	panda::PandaDocument* document;
	QLineEdit* lineEdit;

signals:

public slots:
	void CreateObject();
};

#endif // DATASEDITDIALOG_H
