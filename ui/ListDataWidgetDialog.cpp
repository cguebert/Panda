#include <ui/ListDataWidgetDialog.h>
#include <ui/SimpleDataWidget.h>

template<>
class DataWidgetContainer< QColor > : public BaseOpenDialogObject
{
public:
	typedef QColor value_type;
	QWidget* container;
	QLabel* preview;
	QColor theColor;

	DataWidgetContainer() : container(nullptr), preview(nullptr) {}

	QWidget* createWidgets(QWidget* parent, bool readOnly)
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
		return container;
	}
	QWidget* createWidgets(BaseDataWidget* parent, bool readOnly)
	{
		QWidget* parentWidget = dynamic_cast<QWidget*>(parent);
		QWidget* widget = createWidgets(parentWidget, readOnly);
		QObject::connect(this, SIGNAL(editingFinished()), parent, SLOT(setWidgetDirty()) );
		return widget;
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
		QColor tmp = QColorDialog::getColor(theColor, container, "", QColorDialog::ShowAlphaChannel);
		if(tmp.isValid())
		{
			theColor = tmp;
			emit editingFinished();
		}
	}
	void updatePreview()
	{
		QPixmap pixmap(50, 25);
		pixmap.fill(theColor);
		preview->setPixmap(pixmap);
	}
};

//***************************************************************//

template <bool openMode>
class file_data_widget_container : public BaseOpenDialogObject
{
public:
	typedef QString value_type;
	QWidget* container;
	QLineEdit* lineEdit;

	file_data_widget_container() : container(nullptr), lineEdit(nullptr) {}

	QWidget* createWidgets(QWidget* parent, bool readOnly)
	{
		container = new QWidget(parent);
		lineEdit = new QLineEdit();

		QPushButton* pushButton = new QPushButton("...");
		pushButton->setEnabled(!readOnly);
		pushButton->setMaximumWidth(40);

		QHBoxLayout* layout = new QHBoxLayout(container);
		layout->setMargin(0);
		layout->addWidget(lineEdit, 1);
		layout->addWidget(pushButton);
		container->setLayout(layout);

		QObject::connect(lineEdit, SIGNAL(editingFinished()), this, SIGNAL(editingFinished()));
		QObject::connect(pushButton, SIGNAL(clicked()), this, SLOT(onShowDialog()) );
		return container;
	}
	QWidget* createWidgets(BaseDataWidget* parent, bool readOnly)
	{
		QWidget* parentWidget = dynamic_cast<QWidget*>(parent);
		QWidget* widget = createWidgets(parentWidget, readOnly);
		QObject::connect(this, SIGNAL(editingFinished()), parent, SLOT(setWidgetDirty()) );
		return widget;
	}
	void readFromData(const value_type& v)
	{
		lineEdit->setText(v);
	}
	void writeToData(value_type& v)
	{
		v = lineEdit->text();
	}
	virtual void onShowDialog()
	{
		const QString& v = lineEdit->text();
		QDir dir(v);
		QString r = dir.path();
		if(openMode)
			r = QFileDialog::getOpenFileName(container, container->tr("Open file"), v);
		else
			r = QFileDialog::getSaveFileName(container, container->tr("Save file"), v);
		if(!r.isEmpty())
		{
			lineEdit->setText(r);
			emit editingFinished();
		}
	}
	void updatePreview()
	{
	}
};

//***************************************************************//

class font_data_widget_container : public BaseOpenDialogObject
{
public:
	typedef QString value_type;
	QWidget* container;
	QLabel* preview;
	QFont theFont;

	font_data_widget_container() : container(nullptr), preview(nullptr) {}

	QWidget* createWidgets(QWidget* parent, bool readOnly)
	{
		container = new QWidget(parent);
		preview = new QLabel("Test");

		QPushButton* pushButton = new QPushButton("...");
		pushButton->setEnabled(!readOnly);
		pushButton->setMaximumWidth(40);

		QHBoxLayout* layout = new QHBoxLayout(container);
		layout->setMargin(0);
		layout->addWidget(preview, 1);
		layout->addWidget(pushButton);
		container->setLayout(layout);

		QObject::connect(pushButton, SIGNAL(clicked()), this, SLOT(onShowDialog()) );
		return container;
	}
	QWidget* createWidgets(BaseDataWidget* parent, bool readOnly)
	{
		QWidget* parentWidget = dynamic_cast<QWidget*>(parent);
		QWidget* widget = createWidgets(parentWidget, readOnly);
		QObject::connect(this, SIGNAL(editingFinished()), parent, SLOT(setWidgetDirty()) );
		return widget;
	}
	void readFromData(const value_type& v)
	{
		theFont.fromString(v);
		updatePreview();
	}
	void writeToData(value_type& v)
	{
		v = theFont.toString();
		updatePreview();
	}
	virtual void onShowDialog()
	{
		bool ok;
		QFont tmpFont = QFontDialog::getFont(&ok, theFont, container);
		if(ok)
		{
			theFont = tmpFont;
			emit editingFinished();
		}
	}
	void updatePreview()
	{
		QFont tmpFont = theFont;
		tmpFont.setPointSize(12);
		preview->setFont(tmpFont);
		preview->setText(theFont.family());
	}
};

//***************************************************************//

Creator<DataWidgetFactory, SimpleDataWidget<QColor> > DWClass_color("default",true);
Creator<DataWidgetFactory, SimpleDataWidget<QString, file_data_widget_container<true> > > DWClass_file_open("open file",true);
Creator<DataWidgetFactory, SimpleDataWidget<QString, file_data_widget_container<false> > > DWClass_file_save("save file",true);
Creator<DataWidgetFactory, SimpleDataWidget<QString, font_data_widget_container> > DWClass_font("font",true);

Creator<DataWidgetFactory, OpenDialogDataWidget<QVector<QColor>, ListDataWidgetDialog<QVector<QColor>> > > DWClass_colors_list("default",true);
Creator<DataWidgetFactory, OpenDialogDataWidget<QVector<QString>, ListDataWidgetDialog<QVector<QString>, file_data_widget_container<true> > > > DWClass_files_list_open("open file",true);
Creator<DataWidgetFactory, OpenDialogDataWidget<QVector<QString>, ListDataWidgetDialog<QVector<QString>, file_data_widget_container<false> > > > DWClass_files_list_save("save file",true);
Creator<DataWidgetFactory, OpenDialogDataWidget<QVector<QString>, ListDataWidgetDialog<QVector<QString>, font_data_widget_container> > > DWClass_fonts_list("font",true);
