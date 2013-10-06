#include <ui/ListDataWidgetDialog.h>
#include <ui/SimpleDataWidget.h>

#include <panda/types/Gradient.h>

class ObjectWithPreview
{
public:
	virtual void draw(QPainter& painter, QSize size) = 0;
};

class PreviewView : public QWidget
{
public:
	PreviewView(ObjectWithPreview* obj) : object(obj) {}
	void paintEvent(QPaintEvent*)
	{
		QStylePainter painter(this);
		object->draw(painter, size());
	}

protected:
	ObjectWithPreview* object;
};

template<>
class DataWidgetContainer< QColor > : public BaseOpenDialogObject, public ObjectWithPreview
{
public:
	typedef QColor value_type;
	QWidget* container;
	PreviewView* preview;
	QColor theColor;

	DataWidgetContainer() : container(nullptr), preview(nullptr) {}

	QWidget* createWidgets(QWidget* parent, bool readOnly)
	{
		container = new QWidget(parent);
		preview = new PreviewView(this);

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
		preview->update();
	}
	void draw(QPainter& painter, QSize size)
	{
		static QPixmap pm;
		if(pm.isNull())
		{
			int s = ceil(size.height() / 2.0);
			pm = QPixmap(2*s, 2*s);
			QColor c1 = container->palette().color(QPalette::Midlight);
			QColor c2 = container->palette().color(QPalette::Dark);
			QPainter pmp(&pm);
			pmp.fillRect(0, 0, s, s, c1);
			pmp.fillRect(s, s, s, s, c1);
			pmp.fillRect(0, s, s, s, c2);
			pmp.fillRect(s, 0, s, s, c2);
		}
		painter.fillRect(0, 0, size.width(), size.height(), QBrush(pm));
		painter.fillRect(0, 0, size.width(), size.height(), QBrush(theColor));
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

class font_data_widget_container : public BaseOpenDialogObject, public ObjectWithPreview
{
public:
	typedef QString value_type;
	QWidget* container;
	PreviewView* preview;
	QFont theFont;

	font_data_widget_container() : container(nullptr), preview(nullptr) {}

	QWidget* createWidgets(QWidget* parent, bool readOnly)
	{
		container = new QWidget(parent);
		preview = new PreviewView(this);

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
		preview->update();
	}
	void draw(QPainter& painter, QSize size)
	{
		QFont tmpFont = theFont;
		tmpFont.setPointSize(10);
		painter.setFont(tmpFont);
		painter.drawText(QRectF(QPoint(), size), Qt::AlignVCenter | Qt::TextDontClip, theFont.family());
	}
};

//***************************************************************//
using panda::types::Gradient;

template<>
class DataWidgetContainer< Gradient > : public BaseOpenDialogObject, public ObjectWithPreview
{
public:
	typedef Gradient value_type;
	QWidget* container;
	PreviewView* preview;
	Gradient theGradient;

	DataWidgetContainer() : container(nullptr), preview(nullptr) {}

	QWidget* createWidgets(QWidget* parent, bool readOnly)
	{
		container = new QWidget(parent);
		preview = new PreviewView(this);

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
		if(theGradient != v)
		{
			theGradient = v;
			updatePreview();
		}
	}
	void writeToData(value_type& v)
	{
		if(v != theGradient)
		{
			v = theGradient;
			updatePreview();
		}
	}
	virtual void onShowDialog()
	{
	}
	void updatePreview()
	{
		preview->update();
	}
	void draw(QPainter& painter, QSize size)
	{
		static QPixmap pm;
		if(pm.isNull())
		{
			int s = ceil(size.height() / 2.0);
			pm = QPixmap(2*s, 2*s);
			QColor c1 = container->palette().color(QPalette::Midlight);
			QColor c2 = container->palette().color(QPalette::Dark);
			QPainter pmp(&pm);
			pmp.fillRect(0, 0, s, s, c1);
			pmp.fillRect(s, s, s, s, c1);
			pmp.fillRect(0, s, s, s, c2);
			pmp.fillRect(s, 0, s, s, c2);
		}
		painter.fillRect(0, 0, size.width(), size.height(), QBrush(pm));

		QLinearGradient grad(0, 0, size.width(), 0);
		grad.setStops(theGradient.getStops());
		painter.fillRect(0, 0, size.width(), size.height(), QBrush(grad));
	}
};

//***************************************************************//

Creator<DataWidgetFactory, SimpleDataWidget<QColor> > DWClass_color("default",true);
Creator<DataWidgetFactory, SimpleDataWidget<QString, file_data_widget_container<true> > > DWClass_file_open("open file",true);
Creator<DataWidgetFactory, SimpleDataWidget<QString, file_data_widget_container<false> > > DWClass_file_save("save file",true);
Creator<DataWidgetFactory, SimpleDataWidget<QString, font_data_widget_container> > DWClass_font("font",true);
Creator<DataWidgetFactory, SimpleDataWidget<Gradient> > DWClass_gradient("default",true);

Creator<DataWidgetFactory, OpenDialogDataWidget<QVector<QColor>, ListDataWidgetDialog<QVector<QColor> > > > DWClass_colors_list("default",true);
Creator<DataWidgetFactory, OpenDialogDataWidget<QVector<QString>, ListDataWidgetDialog<QVector<QString>, file_data_widget_container<true> > > > DWClass_files_list_open("open file",true);
Creator<DataWidgetFactory, OpenDialogDataWidget<QVector<QString>, ListDataWidgetDialog<QVector<QString>, file_data_widget_container<false> > > > DWClass_files_list_save("save file",true);
Creator<DataWidgetFactory, OpenDialogDataWidget<QVector<QString>, ListDataWidgetDialog<QVector<QString>, font_data_widget_container> > > DWClass_fonts_list("font",true);
Creator<DataWidgetFactory, OpenDialogDataWidget<QVector<QColor>, ListDataWidgetDialog<QVector<QColor> > > > DWClass_gradients_list("default",true);
