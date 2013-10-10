#include <ui/ListDataWidgetDialog.h>
#include <ui/AnimationDataWidgetDialog.h>
#include <ui/SimpleDataWidget.h>

#include <panda/types/Animation.h>

using panda::types::Animation;

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
class FileDataWidgetContainer : public BaseOpenDialogObject
{
public:
	typedef QString value_type;
	QWidget* container;
	QLineEdit* lineEdit;

	FileDataWidgetContainer() : container(nullptr), lineEdit(nullptr) {}

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

class FontDataWidgetContainer : public BaseOpenDialogObject, public ObjectWithPreview
{
public:
	typedef QString value_type;
	QWidget* container;
	PreviewView* preview;
	QFont theFont;

	FontDataWidgetContainer() : container(nullptr), preview(nullptr) {}

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

template<class T>
class VectorDataTrait< Animation<T> >
{
public:
	typedef Animation<T> animation_type;
	typedef T row_type;

	enum { is_vector = 1 };
	enum { is_single = 0 };
	static int size(const animation_type& a) { return a.size(); }
	static QStringList header(const animation_type&) { return QStringList{}; }
	static const row_type* get(const animation_type&, int) { return nullptr; }
	static void set(animation_type&, const row_type&, int) { }
	static void resize(animation_type&, int) { }
};

//***************************************************************//

Creator<DataWidgetFactory, SimpleDataWidget<QColor> > DWClass_color("default",true);
Creator<DataWidgetFactory, SimpleDataWidget<QString, FileDataWidgetContainer<true> > > DWClass_file_open("open file",true);
Creator<DataWidgetFactory, SimpleDataWidget<QString, FileDataWidgetContainer<false> > > DWClass_file_save("save file",true);
Creator<DataWidgetFactory, SimpleDataWidget<QString, FontDataWidgetContainer> > DWClass_font("font",true);

Creator<DataWidgetFactory, OpenDialogDataWidget<QVector<QColor>, ListDataWidgetDialog<QVector<QColor> > > > DWClass_colors_list("default",true);
Creator<DataWidgetFactory, OpenDialogDataWidget<QVector<QString>, ListDataWidgetDialog<QVector<QString>, FileDataWidgetContainer<true> > > > DWClass_files_list_open("open file",true);
Creator<DataWidgetFactory, OpenDialogDataWidget<QVector<QString>, ListDataWidgetDialog<QVector<QString>, FileDataWidgetContainer<false> > > > DWClass_files_list_save("save file",true);
Creator<DataWidgetFactory, OpenDialogDataWidget<QVector<QString>, ListDataWidgetDialog<QVector<QString>, FontDataWidgetContainer> > > DWClass_fonts_list("font",true);

Creator<DataWidgetFactory, OpenDialogDataWidget<Animation<double>, AnimationDataWidgetDialog<Animation<double>, DoubleDataWidgetContainer > > > DWClass_reals_animation("default",true);
Creator<DataWidgetFactory, OpenDialogDataWidget<Animation<QPointF>, AnimationDataWidgetDialog<Animation<QPointF>, PointDataWidgetContainer > > > DWClass_points_animation("default",true);
Creator<DataWidgetFactory, OpenDialogDataWidget<Animation<QColor>, AnimationDataWidgetDialog<Animation<QColor> > > > DWClass_colors_animation("default",true);
