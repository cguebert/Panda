#ifndef CHOOSEWIDGETDIALOG_H
#define CHOOSEWIDGETDIALOG_H

#include <QDialog>

namespace panda
{
	class BaseData;
}

class QLabel;
class QTextEdit;
class QComboBox;

class ChooseWidgetDialog : public QDialog
{
	Q_OBJECT
public:
	explicit ChooseWidgetDialog(panda::BaseData*, QWidget* parent = nullptr);

	QSize sizeHint() const;

protected:
	panda::BaseData* m_data;
	QComboBox* m_types;
	QLabel* m_format;
	QTextEdit* m_parameters;

public slots:
	void changedType(const QString&);
	void changeData();
};

#endif // CHOOSEWIDGETDIALOG_H
