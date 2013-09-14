#include <ui/StructDataWidget.h>
#include <ui/SimpleDataWidget.h>
#include <helper/Factory.h>

template<>
class data_widget_container< QColor > : public BaseOpenDialogObject
{
public:
	typedef QColor value_type;
	QWidget* container;
	QLabel* preview;
	QColor theColor;

	data_widget_container() : container(nullptr), preview(nullptr) {}

	QWidget* createWidgets(BaseDataWidget* parent, bool readOnly)
	{
		container = new QWidget(parent);
		preview = new QLabel();

		QPushButton* pushButton = new QPushButton("...");
		pushButton->setEnabled(!readOnly);
		pushButton->setMaximumWidth(40);

		QHBoxLayout* layout = new QHBoxLayout(container);
		layout->setMargin(0);
		layout->addWidget(preview, 1);
		layout->addWidget(pushButton);
		container->setLayout(layout);

		QObject::connect(pushButton, SIGNAL(clicked()), this, SLOT(onShowDialog()) );
		QObject::connect(this, SIGNAL(editingFinished()), parent, SLOT(setWidgetDirty()) );
		return container;
	}
	void readFromData(const value_type& v)
	{
		if(theColor != v)
		{
			theColor = v;
			updatePreview();
		}
	}
	void writeToData(value_type& v)
	{
		if(v != theColor)
		{
			v = theColor;
			updatePreview();
		}
	}
	virtual void onShowDialog()
	{
		QColor tmp = QColorDialog::getColor(theColor, container);
		if(tmp.isValid())
		{
			theColor = tmp;
			emit editingFinished();
		}
	}
	void updatePreview()
	{
		QPixmap pixmap(50, 50);
		pixmap.fill(theColor);
		preview->setPixmap(pixmap);
	}
};

//***************************************************************//

Creator<DataWidgetFactory, SimpleDataWidget<QColor> > DWClass_color("default",true);
Creator<DataWidgetFactory, OpenDialogDataWidget<QPointF> > DWClass_point("default",true);
Creator<DataWidgetFactory, OpenDialogDataWidget<QRectF> > DWClass_rect("default",true);

Creator<DataWidgetFactory, OpenDialogDataWidget<QVector<int> > > DWClass_ints_list("default",true);
Creator<DataWidgetFactory, OpenDialogDataWidget<QVector<double> > > DWClass_doubles_list("default",true);
Creator<DataWidgetFactory, OpenDialogDataWidget<QVector<QPointF> > > DWClass_points_list("default",true);
Creator<DataWidgetFactory, OpenDialogDataWidget<QVector<QRectF> > > DWClass_rects_list("default",true);
Creator<DataWidgetFactory, OpenDialogDataWidget<QVector<QString> > > DWClass_strings_list("default",true);
