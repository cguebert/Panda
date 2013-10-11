#ifndef ANIMATIONDATAWIDGETDIALOG_H
#define ANIMATIONDATAWIDGETDIALOG_H

#include <panda/Data.h>
#include <QtWidgets>

class BaseAnimationDataWidgetDialog : public QDialog
{
	Q_OBJECT
public:
	BaseAnimationDataWidgetDialog(QWidget* parent)
		: QDialog(parent)
	{}

signals:

public slots:
	virtual void refreshPreviews() {}
	virtual void resizeValue() {}
	virtual void changeExtend(int) {}
	virtual void changeInterpolation(int) {}
};

template<class T, class Container = DataWidgetContainer< T::value_type > >
class AnimationDataWidgetDialog : public BaseAnimationDataWidgetDialog
{
protected:
	typedef T animation_type;
	typedef panda::Data<T> data_type;
	typedef typename animation_type::value_type value_type;

	typedef QSharedPointer<Container> ContainerPtr;

	bool readOnly;
	QScrollArea* scrollArea;
	QGridLayout* gridLayout;
	QWidget* resizeWidget;
	QSpinBox* resizeSpinBox;
	QVector<ContainerPtr> containers;
	QVector<QLineEdit*> lineEdits;
	QComboBox *extendBox, *interpolationBox;

public:
	AnimationDataWidgetDialog(QWidget* parent, const data_type& d, bool readOnly)
		: BaseAnimationDataWidgetDialog(parent)
		, readOnly(readOnly)
		, scrollArea(nullptr)
		, gridLayout(nullptr)
		, resizeWidget(nullptr)
		, resizeSpinBox(nullptr)
	{
		QVBoxLayout* mainLayout = new QVBoxLayout();

		if(!readOnly)
		{
			resizeWidget = new QWidget(this);
			QLabel* resizeLabel = new QLabel(tr("Size"));
			resizeSpinBox = new QSpinBox();
			resizeSpinBox->setMinimum(0);
			resizeSpinBox->setMaximum(65535);
			QPushButton* resizeButton = new QPushButton(tr("Resize"));
			QHBoxLayout* resizeLayout = new QHBoxLayout;
			resizeLayout->setMargin(0);
			resizeLayout->addWidget(resizeLabel);
			resizeLayout->addWidget(resizeSpinBox, 1);
			resizeLayout->addWidget(resizeButton);
			resizeWidget->setLayout(resizeLayout);
			mainLayout->addWidget(resizeWidget);

			connect(resizeButton, SIGNAL(clicked()), this, SLOT(resizeValue()));
		}

		scrollArea = new QScrollArea(this);
		scrollArea->setFrameShape(QFrame::NoFrame);
		scrollArea->setWidgetResizable(true);
		QWidget *layoutWidget = new QWidget(scrollArea);
		gridLayout = new QGridLayout(layoutWidget);
		gridLayout->setMargin(0);
		scrollArea->setWidget(layoutWidget);
		mainLayout->addWidget(scrollArea);

		QLabel* extendLabel = new QLabel(tr("Extend method:"));
		extendBox = new QComboBox;
		extendBox->addItem("Pad");
		extendBox->addItem("Repeat");
		extendBox->addItem("Reflect");
		QHBoxLayout* extendLayout = new QHBoxLayout;
		extendLayout->addWidget(extendLabel);
		extendLayout->addWidget(extendBox);
		mainLayout->addLayout(extendLayout);

		QLabel* interpolationLabel = new QLabel(tr("Interpolation method:"));
		interpolationBox = new QComboBox;
		const char* interpolationTypes[] = {"Linear",
						 "InQuad", "OutQuad", "InOutQuad", "OutInQuad",
						 "InCubic", "OutCubic", "InOutCubic", "OutInCubic",
						 "InQuart", "OutQuart", "InOutQuart", "OutInQuart",
						 "InQuint", "OutQuint", "InOutQuint", "OutInQuint",
						 "InSine", "OutSine", "InOutSine", "OutInSine",
						 "InExpo", "OutExpo", "InOutExpo", "OutInExpo",
						 "InCirc", "OutCirc", "InOutCirc", "OutInCirc",
						 "InElastic", "OutElastic", "InOutElastic", "OutInElastic",
						 "InBack", "OutBack", "InOutBack", "OutInBack",
						 "InBounce", "OutBounce", "InOutBounce", "OutInBounce",
						 "InCurve", "OutCurve", "SineCurve", "CosineCurve"};
		for(auto type : interpolationTypes)
			interpolationBox->addItem(type);
		QHBoxLayout* interpolationLayout = new QHBoxLayout;
		interpolationLayout->addWidget(interpolationLabel);
		interpolationLayout->addWidget(interpolationBox);
		mainLayout->addLayout(interpolationLayout);

		QPushButton* okButton = new QPushButton(tr("Ok"), this);
		okButton->setDefault(true);
		connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
		QPushButton* cancelButton = new QPushButton(tr("Cancel"), this);
		connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
		QHBoxLayout* buttonsLayout = new QHBoxLayout;
		buttonsLayout->addStretch();
		buttonsLayout->addWidget(okButton);
		buttonsLayout->addWidget(cancelButton);
		mainLayout->addLayout(buttonsLayout);

		setLayout(mainLayout);

		setWindowTitle(d.getName() + (readOnly ? tr(" (read-only)") : ""));
	}

	void updateTable(const animation_type& anim)
	{
		auto stops = anim.getStops();
		int nb = stops.size();

		for(int i=0; i<nb; ++i)
		{
			lineEdits[i]->setText(QString::number(stops[i].first));
			containers[i]->readFromData(stops[i].second);
		}
	}

	virtual void readFromData(const animation_type& anim)
	{
		resizeContainers(anim.size());
		updateTable(anim);

		extendBox->setCurrentIndex(anim.getExtend());
		interpolationBox->setCurrentIndex(anim.getInterpolation());

		if(resizeSpinBox)
			resizeSpinBox->setValue(containers.size());
	}

	animation_type readFromTable()
	{
		animation_type anim;

		int nb = containers.size();

		for(int i=0; i<nb; ++i)
		{
			value_type val;
			containers[i]->writeToData(val);
			double key = lineEdits[i]->text().toDouble();
			anim.add(key, val);
		}

		anim.setExtend(extendBox->currentIndex());
		anim.setInterpolation(interpolationBox->currentIndex());
		return anim;
	}

	virtual void writeToData(animation_type& anim)
	{
		anim = readFromTable();
	}

	void resizeContainers(int nb)
	{
		int oldSize = containers.size();
		if(oldSize == nb)
			return;

		if(oldSize > nb)
		{	// Removing
			containers.resize(nb);
			lineEdits.resize(nb);
			for(int i=oldSize-1; i>=nb; --i)
			{
				QLayoutItem* key = gridLayout->itemAtPosition(i, QFormLayout::LabelRole);
				QLayoutItem* field = gridLayout->itemAtPosition(i, QFormLayout::FieldRole);
				key->widget()->hide();
				field->widget()->hide();
				gridLayout->removeItem(key);
				gridLayout->removeItem(field);
				delete key;
				delete field;
			}
		}
		else
		{	// Adding
			for(int i=oldSize; i<nb; ++i)
			{
				ContainerPtr container = ContainerPtr(new Container());
				containers.push_back(container);
				QWidget* widget = container->createWidgets(this, readOnly);
				container->updatePreview();
				connect(container.data(), SIGNAL(editingFinished()), this, SLOT(refreshPreviews()));
				QLineEdit* lineEdit = new QLineEdit("0.0");
				lineEdits.push_back(lineEdit);
				gridLayout->addWidget(lineEdit, i, 0);
				gridLayout->addWidget(widget, i, 1);
			}
		}

		gridLayout->setColumnStretch(1, 1);
		gridLayout->setColumnMinimumWidth(1, 150);
	}

	virtual void resizeValue()
	{
		int nb = resizeSpinBox->value();
		resizeContainers(nb);
	}

	virtual void refreshPreviews()
	{
		foreach(ContainerPtr container, containers)
			container->updatePreview();
	}

	virtual void changeExtend(int)
	{

	}

	virtual void changeInterpolation(int)
	{

	}
};

//***************************************************************//

class DoubleDataWidgetContainer : public QObject
{
	Q_OBJECT
signals:
	void editingFinished();

protected:
	typedef double value_type;
	QLineEdit* lineEdit;

public:
	DoubleDataWidgetContainer() : lineEdit(nullptr) {}

	QWidget* createWidgets(QWidget* parent, bool readOnly)
	{
		lineEdit = new QLineEdit("0.0", parent);
		lineEdit->setEnabled(!readOnly);
		return lineEdit;
	}
	void readFromData(const value_type& v)
	{
		QString t = lineEdit->text();
		value_type n = t.toDouble();
		if (v != n || t.isEmpty())
			lineEdit->setText(QString::number(v));
	}
	void writeToData(value_type& v)
	{
		bool ok;
		value_type n = lineEdit->text().toDouble(&ok);
		if(ok)
			v = n;
	}
	void updatePreview() {}
};

//***************************************************************//

class PointDataWidgetContainer : public QObject
{
	Q_OBJECT
signals:
	void editingFinished();

protected:
	typedef QPointF value_type;
	QWidget* container;
	QLineEdit *lineEditX, *lineEditY;

public:
	PointDataWidgetContainer() : lineEditX(nullptr), lineEditY(nullptr) {}

	QWidget* createWidgets(QWidget* parent, bool readOnly)
	{
		container = new QWidget(parent);
		lineEditX = new QLineEdit("0.0", parent);
		lineEditX->setEnabled(!readOnly);

		lineEditY = new QLineEdit("0.0", parent);
		lineEditY->setEnabled(!readOnly);

		QHBoxLayout* layout = new QHBoxLayout(container);
		layout->setMargin(0);
		layout->addWidget(lineEditX);
		layout->addWidget(lineEditY);
		container->setLayout(layout);

		return container;
	}
	void readFromData(const value_type& v)
	{
		QString tx = lineEditX->text();
		QString ty = lineEditY->text();
		double x = tx.toDouble();
		double y = ty.toDouble();
		if(v.x() != x || tx.isEmpty())
			lineEditX->setText(QString::number(v.x()));
		if(v.y() != y || ty.isEmpty())
			lineEditY->setText(QString::number(v.y()));
	}
	void writeToData(value_type& v)
	{
		double x = lineEditX->text().toDouble();
		double y = lineEditY->text().toDouble();
		v = QPointF(x, y);
	}
	void updatePreview() {}
};

#endif // ANIMATIONDATAWIDGETDIALOG_H
