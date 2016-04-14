#include <ui/widget/DataWidgetFactory.h>
#include <ui/widget/OpenDialogDataWidget.h>
#include <ui/widget/ListDataWidgetDialog.h>
#include <ui/widget/SimpleDataWidget.h>
#include <panda/helper/Font.h>

#include <QtWidgets>

namespace
{

using FontPair = std::pair<std::string, int>; // Path & index of the font in the file
#ifdef Q_OS_WIN32
using FontsMap = QMap<QString, FontPair>;
const FontsMap& getFontsMap()
{
	static FontsMap fontsMap;
	if (fontsMap.empty())
	{
		QSettings settings("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Fonts", QSettings::NativeFormat);
		QStringList list = settings.allKeys();
		for (const auto& key : list)
		{ 
			auto faces = key.split(" & ");
			auto& last = faces.back();
			last = last.left(last.indexOf(" ("));

			auto text = settings.value(key).toString();
			for (int i = 0; i < faces.size(); ++i)
				fontsMap[faces[i]] = std::make_pair(text.toStdString(), i);
		}
	}

	return fontsMap;
}
#endif

FontPair getFontFilename(QFont font)
{
#ifdef Q_OS_WIN32
	QString family = font.family();
	QString familyPlus = family + (font.weight() == 75 ? " Bold" : "") + (font.italic() ? " Italic" : "");
	const auto& fontsMap = getFontsMap();
	auto fp = fontsMap.value(familyPlus);
	if (!fp.first.empty())
		return fp;
	return fontsMap.value(family);
#else
	QFontInfo info(font);
	QFont realFont(info.family());
	return std::make_pair(realFont.rawName().toStdString(), 0);
#endif
}

}

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
	QString filter;

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
		filter = parent->getParameters();
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
		if (openMode)
			r = QFileDialog::getOpenFileName(container, container->tr("Open file"), v, filter, nullptr, QFileDialog::HideNameFilterDetails);
		else
			r = QFileDialog::getSaveFileName(container, container->tr("Save file"), v, filter, nullptr, QFileDialog::HideNameFilterDetails);
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

class DirectoryDataWidgetContainer : public BaseOpenDialogObject
{
protected:
	typedef std::string value_type;
	QWidget* container;
	QLineEdit* lineEdit;

public:
	DirectoryDataWidgetContainer() : container(nullptr), lineEdit(nullptr) {}

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
		QObject::connect(pushButton, SIGNAL(clicked()), this, SLOT(onShowDialog()));
		return container;
	}
	QWidget* createWidgets(BaseDataWidget* parent, bool readOnly)
	{
		QWidget* parentWidget = dynamic_cast<QWidget*>(parent);
		QWidget* widget = createWidgets(parentWidget, readOnly);
		QObject::connect(this, SIGNAL(editingFinished()), parent, SLOT(setWidgetDirty()));
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
		r = QFileDialog::getExistingDirectory(container, container->tr("Choose directory"), v);
		if (!r.isEmpty())
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
		theFont = QFont();
		panda::helper::Font f(v);

		theFont.setFamily(QString::fromStdString(f.name));
		theFont.setPointSize(f.pointSize);
		theFont.setWeight(f.weight);
		theFont.setItalic(f.italic);
		updatePreview();
	}
	void writeToData(value_type& v)
	{
		panda::helper::Font font;
		font.name = theFont.family().toStdString();
		std::tie(font.path, font.faceIndex) = getFontFilename(theFont);
		font.pointSize = theFont.pointSize();
		font.italic = theFont.italic();
		font.weight = theFont.weight();
		v = font.toString();
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
		textEdit->setTabStopWidth(20);
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

	QSize sizeHint() const
	{
		return QSize(500, 400);
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

template<>
static QString FlatDataTrait<QString>::toString(const value_type& d)
{ return d; }

//****************************************************************************//

RegisterWidget<SimpleDataWidget<std::string> > DWClass_string("default");
RegisterWidget<SimpleDataWidget<std::string, FileDataWidgetContainer<true> > > DWClass_file_open("open file");
RegisterWidget<SimpleDataWidget<std::string, FileDataWidgetContainer<false> > > DWClass_file_save("save file");
RegisterWidget<SimpleDataWidget<std::string, DirectoryDataWidgetContainer > > DWClass_file_directory("directory");
RegisterWidget<SimpleDataWidget<std::string, FontDataWidgetContainer> > DWClass_font("font");
RegisterWidget<SimpleDataWidget<std::string, MultilineDataWidgetContainer> > DWClass_multiline("multiline");

//RegisterWidget<OpenDialogDataWidget<std::vector<QString>, TableDataWidgetDialog<std::vector<QString> > > > DWClass_strings_list("default");
RegisterWidget<OpenDialogDataWidget<std::vector<std::string>, ListDataWidgetDialog<std::vector<std::string> > > > DWClass_strings_list_generic("generic");
