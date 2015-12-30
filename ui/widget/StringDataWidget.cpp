#include <ui/widget/DataWidgetFactory.h>
#include <ui/widget/OpenDialogDataWidget.h>
#include <ui/widget/ListDataWidgetDialog.h>
#include <ui/widget/SimpleDataWidget.h>

#include <QtWidgets>

template<>
class DataWidgetContainer< std::string >
{
protected:
	typedef std::string value_type;
	QLineEdit* lineEdit;

public:
	DataWidgetContainer() : lineEdit(nullptr) {}

	QWidget* createWidgets(BaseDataWidget* parent, bool readOnly)
	{
		lineEdit = new QLineEdit(parent);
		lineEdit->setEnabled(!readOnly);
		QObject::connect(lineEdit, SIGNAL(editingFinished()), parent, SLOT(setWidgetDirty()) );
		return lineEdit;
	}
	void readFromData(const value_type& v)
	{
		auto qVal = QString::fromStdString(v);
		if (lineEdit->text() != qVal)
			lineEdit->setText(qVal);
	}
	void writeToData(value_type& v)
	{
		v = lineEdit->text().toStdString();
	}
};

//****************************************************************************//

template <bool openMode>
class FileDataWidgetContainer : public BaseOpenDialogObject
{
protected:
	typedef std::string value_type;
	QWidget* container;
	QLineEdit* lineEdit;

public:
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
		lineEdit->setText(QString::fromStdString(v));
	}
	void writeToData(value_type& v)
	{
		v = lineEdit->text().toStdString();
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

//****************************************************************************//

class FontDataWidgetContainer : public BaseOpenDialogObject, public ObjectWithPreview
{
protected:
	typedef std::string value_type;
	QWidget* container;
	PreviewView* preview;
	QFont theFont;

public:
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
		theFont.fromString(QString::fromStdString(v));
		updatePreview();
	}
	void writeToData(value_type& v)
	{
		v = theFont.toString().toStdString();
		updatePreview();
	}
	virtual void onShowDialog()
	{
		bool ok;
		QFont tmpFont = QFontDialog::getFont(&ok, theFont);
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

//****************************************************************************//

class MultilineDataWidgetDialog : public QDialog
{
protected:
	typedef std::string value_type;
	QTextEdit* textEdit;

public:
	MultilineDataWidgetDialog(QWidget* parent, const value_type& v, bool readOnly)
		: QDialog(parent)
		, textEdit(nullptr)
	{
		QVBoxLayout* mainLayout = new QVBoxLayout();

		textEdit = new QTextEdit;
		textEdit->setPlainText(QString::fromStdString(v));
		textEdit->setEnabled(!readOnly);
		mainLayout->addWidget(textEdit);

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

		setWindowTitle(tr("Multiline edit") + (readOnly ? tr(" (read-only)") : ""));
	}

	std::string getText()
	{
		return textEdit->toPlainText().toStdString();
	}
};

class MultilineDataWidgetContainer : public BaseOpenDialogObject
{
protected:
	typedef std::string value_type;
	QWidget* container;
	QLabel* label;
	std::string theText;
	bool isReadOnly;

public:
	MultilineDataWidgetContainer() : container(nullptr), label(nullptr) {}

	QWidget* createWidgets(QWidget* parent, bool readOnly)
	{
		isReadOnly = readOnly;
		container = new QWidget(parent);
		label = new QLabel();
		label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

		QPushButton* pushButton = new QPushButton("...");
		pushButton->setEnabled(!readOnly);
		pushButton->setMaximumWidth(40);

		QHBoxLayout* layout = new QHBoxLayout(container);
		layout->setMargin(0);
		layout->addWidget(label, 1);
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
		theText = v;
		label->setText(QString::fromStdString(v));
	}
	void writeToData(value_type& v)
	{
		v = theText;
	}
	virtual void onShowDialog()
	{
		MultilineDataWidgetDialog dlg(container, theText, isReadOnly);
		if(dlg.exec() == QDialog::Accepted)
		{
			theText = dlg.getText();
			label->setText(QString::fromStdString(theText));
			emit editingFinished();
		}
	}
	void updatePreview()
	{
	}
};

//****************************************************************************//

RegisterWidget<SimpleDataWidget<std::string> > DWClass_string("default");
RegisterWidget<SimpleDataWidget<std::string, FileDataWidgetContainer<true> > > DWClass_file_open("open file");
RegisterWidget<SimpleDataWidget<std::string, FileDataWidgetContainer<false> > > DWClass_file_save("save file");
RegisterWidget<SimpleDataWidget<std::string, FontDataWidgetContainer> > DWClass_font("font");
RegisterWidget<SimpleDataWidget<std::string, MultilineDataWidgetContainer> > DWClass_multiline("multiline");

RegisterWidget<OpenDialogDataWidget<std::vector<std::string>, ListDataWidgetDialog<std::vector<std::string> > > > DWClass_strings_list_generic("generic");
