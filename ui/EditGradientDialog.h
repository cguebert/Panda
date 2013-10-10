#ifndef EDITGRADIENTDIALOG_H
#define EDITGRADIENTDIALOG_H

#include <panda/types/Gradient.h>
#include <QDialog>
#include <QPainterPath>
#include <QVector>

class QLineEdit;
class QPushButton;
class EditGradientDialog;
class ColorPreviewWidget;

class ColorPreviewWidget : public QWidget
{
	Q_OBJECT
public:
	ColorPreviewWidget(QWidget* parent);

	void setColor(QColor color);
	void paintEvent(QPaintEvent *);

protected:
	QColor theColor;
};

class EditGradientView : public QWidget
{
	Q_OBJECT
public:
	EditGradientView(EditGradientDialog* dlg, panda::types::Gradient::GradientStops& stops);

	void paintEvent(QPaintEvent *);
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);

	void setSelected(int);

protected:
	panda::types::Gradient::GradientStops& stops;
	EditGradientDialog* dialog;
	QVector<QPainterPath> paths;
	int selected;
	bool moving;
	int initialMouseX;
	double initialPos, prevPos;
};

class EditGradientDialog : public QDialog
{
	Q_OBJECT
public:
	explicit EditGradientDialog(panda::types::Gradient grad, QWidget *parent = 0);
	panda::types::Gradient getGradient();

public slots:
	void setSelected(int);
	void changeExtend(int);
	void chooseColor();
	void changePosition();
	void addStop();
	void removeStop();
	void positionHasChanged();

protected:
	bool EditGradientDialog::eventFilter(QObject *obj, QEvent *event);

	EditGradientView* view;
	ColorPreviewWidget* colorPreview;
	panda::types::Gradient::GradientStops stops;
	QLineEdit* posEdit;
	QPushButton *removeButton, *colorButton;
	int extend, selected;

public slots:
};

#endif // EDITGRADIENTDIALOG_H
