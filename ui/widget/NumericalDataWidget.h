#pragma once

#include <ui/widget/SimpleDataWidget.h>

#include <QtWidgets>

template <class T>
class SliderDataWidget : public DataWidgetContainer< T >
{
protected:
	typedef T value_type;
	QSlider* slider;
	QLabel* label;
	QWidget* container;
	value_type vMin, vMax, step;
	int sRange;

public:
	SliderDataWidget() : slider(nullptr), label(nullptr), container(nullptr)
	  , vMin(0), vMax(100), step(1), sRange(100) {}

	QWidget* createWidgets(BaseDataWidget* parent, bool readOnly)
	{
		container = new QWidget(parent);

		label = new QLabel(container);

		slider = new QSlider(container);
		slider->setOrientation(Qt::Horizontal);
		slider->setTickInterval(QSlider::NoTicks);
		slider->setEnabled(!readOnly);
		readParameters(parent->getParameters());

		QHBoxLayout* layout = new QHBoxLayout(container);
		layout->setMargin(0);
		layout->addWidget(label, 1);
		layout->addWidget(slider, 5);
		container->setLayout(layout);

		QObject::connect(slider, SIGNAL(valueChanged(int)), parent, SLOT(setWidgetDirty()));
		return container;
	}
	void readParameters(QString wd)
	{
		if(!wd.isEmpty())
		{
			QTextStream stream(&wd);
			stream >> vMin >> vMax >> step;

			if(vMin > vMax)
				std::swap(vMin, vMax);
			if(!vMin && !vMax) vMax = 100;
			step = abs(step);
			if(!step) step = 1;

			sRange = (vMax - vMin) / step;
		}

		slider->setMinimum(0);
		slider->setMaximum(sRange);
		slider->setSingleStep(1);
		slider->setPageStep(qMax(5, sRange/10));
	}
	void readFromData(const value_type& v)
	{
		value_type t = qBound(vMin, v, vMax);
		t = (t - vMin) / step;
		slider->setValue(t);
		label->setText(QString::number(v));
	}
	void writeToData(value_type& v)
	{
		value_type t = slider->value();
		v = t * step + vMin;
		label->setText(QString::number(v));
	}
	static QString GetParametersFormat()
	{
		return "min max step";
	}
};
