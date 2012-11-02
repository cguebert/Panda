#include <QtGui>

#include <ui/QuickCreateDialog.h>

#include <panda/PandaDocument.h>


QuickCreateDialog::QuickCreateDialog(panda::PandaDocument* doc, QWidget *parent)
    : QDialog(parent)
	, document(doc)
{
    QVBoxLayout* vLayout = new QVBoxLayout;



}

void QuickCreateDialog::CreateObject()
{

}
