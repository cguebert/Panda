#ifndef ANIMATIONDATAWIDGETDIALOG_H
#define ANIMATIONDATAWIDGETDIALOG_H

#include <ui/widget/DataWidget.h>
#include <ui/widget/DataWidgetFactory.h>
#include <ui/widget/StructTraits.h>

#include <panda/types/AnimationTraits.h>

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
};

template<class T>
class AnimationDataWidgetDialog : public BaseAnimationDataWidgetDialog
{
protected:
	typedef T animation_type;
	typedef panda::Data<T> data_type;
	typedef typename animation_type::value_type value_type;
	typedef typename animation_type::ValuesList ValuesList;
	typedef VectorDataTrait<ValuesList> values_list_trait;

	typedef DataWidget<value_type> ChildDataWidget;
	typedef std::shared_ptr<ChildDataWidget> DataWidgetPtr;

	bool readOnly;
	QScrollArea* scrollArea;
	QGridLayout* gridLayout;
	QWidget* resizeWidget;
	QSpinBox* resizeSpinBox;
	QVector<DataWidgetPtr> dataWidgets;
	QVector<QLineEdit*> lineEdits;
	DataWidget<animation_type>* parentDW;
	int valueTypeFullId;
	ValuesList valuesCopy;
	QComboBox *extendBox, *interpolationBox;
	const BaseDataWidgetCreator* dataWidgetCreator;

public:
	AnimationDataWidgetDialog(DataWidget<animation_type>* parent, bool readOnly, QString name)
		: BaseAnimationDataWidgetDialog(parent)
		, readOnly(readOnly)
		, scrollArea(nullptr)
		, gridLayout(nullptr)
		, resizeWidget(nullptr)
		, resizeSpinBox(nullptr)
		, parentDW(parent)
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
		const auto interpolationNames = panda::helper::EasingFunctions::TypeNames();
		for(const auto& type : interpolationNames)
			interpolationBox->addItem(QString::fromStdString(type));
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
		setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

		setWindowTitle(name + (readOnly ? tr(" (read-only)") : ""));

		valueTypeFullId = panda::types::DataTrait<value_type>::fullTypeId();
		dataWidgetCreator = DataWidgetFactory::getInstance()->getCreator(valueTypeFullId, parentDW->getWidgetName());
	}

	virtual void readFromData(const animation_type& anim)
	{
		valuesCopy = anim.getValues();
		resize(anim.size());

		auto stops = anim.getStops();
		int nb = stops.size();

		for(int i=0; i<nb; ++i)
		{
			lineEdits[i]->setText(QString::number(stops[i].first));
			dataWidgets[i]->updateWidgetValue();
		}

		extendBox->setCurrentIndex(anim.getExtend());
		interpolationBox->setCurrentIndex(anim.getInterpolation());

		if(resizeSpinBox)
			resizeSpinBox->setValue(dataWidgets.size());
	}

	virtual void writeToData(animation_type& anim)
	{
		anim.clear();

		int nb = dataWidgets.size();

		for(int i=0; i<nb; ++i)
		{
			dataWidgets[i]->updateDataValue();
			float key = lineEdits[i]->text().toFloat();
			anim.add(key, valuesCopy[i]);
		}

		anim.setExtend(extendBox->currentIndex());
		anim.setInterpolation(interpolationBox->currentIndex());
	}

	void resize(int nb)
	{
		int oldSize = dataWidgets.size();

		values_list_trait::resize(valuesCopy, nb);

		// Update value pointers if they changed
		int updateNb = qMin(nb, oldSize);
		for(int i=0; i<updateNb; ++i)
			dataWidgets[i]->changeValuePointer(values_list_trait::get(valuesCopy, i));

		if(oldSize == nb)
			return;

		if(oldSize > nb)
		{	// Removing
			dataWidgets.resize(nb);
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
		else if(dataWidgetCreator)
		{	// Adding
			QString widget = parentDW->getWidgetName(),
					parentName = parentDW->getDisplayName(),
					parameters = parentDW->getParameters();
			for(int i=oldSize; i<nb; ++i)
			{
				QString displayName = parentName + " / " + QString::number(i);
				value_type* pValue = values_list_trait::get(valuesCopy, i);
				BaseDataWidget* baseDataWidget = dataWidgetCreator->create(this, pValue, widget, displayName, parameters);
				ChildDataWidget* dataWidget = dynamic_cast<ChildDataWidget*>(baseDataWidget);
				if(dataWidget)
				{
					dataWidgets.push_back(DataWidgetPtr(dataWidget));
					QWidget* widget = dataWidget->createWidgets(readOnly);
					QLineEdit* lineEdit = new QLineEdit("0.0");
					lineEdit->setEnabled(!readOnly);
					lineEdits.push_back(lineEdit);
					gridLayout->addWidget(lineEdit, i, 0);
					gridLayout->addWidget(widget, i, 1);
				}
				else
					delete baseDataWidget;
			}
		}

		gridLayout->setColumnStretch(1, 1);
		gridLayout->setColumnMinimumWidth(1, 150);
	}

	virtual void resizeValue()
	{
		int nb = resizeSpinBox->value();
		resize(nb);
	}

	static QString GetParametersFormat() { return ParametersFormatHelper<ChildDataWidget>(); }
};

//****************************************************************************//

template<class T>
class VectorDataTrait< panda::types::Animation<T> >
{
public:
	typedef panda::types::Animation<T> animation_type;
	typedef T row_type;

	enum { is_vector = 1 };
	enum { is_single = 0 };
	static int size(const animation_type& a) { return a.size(); }
	static QStringList header(const animation_type&) { return QStringList{}; }
	static const row_type* get(const animation_type&, int) { return nullptr; }
	static row_type* get(animation_type&, int) { return nullptr; }
	static void set(animation_type&, const row_type&, int) { }
	static void resize(animation_type&, int) { }
};

#endif // ANIMATIONDATAWIDGETDIALOG_H
