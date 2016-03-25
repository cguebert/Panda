#include <ui/widget/DataWidgetFactory.h>
#include <ui/widget/OpenDialogDataWidget.h>
#include <ui/widget/SimpleDataWidget.h>

#include <Qsci/qsciscintilla.h>
#include <Qsci/qscilexercpp.h>
#include <QtWidgets>

class SourceCodeDataWidgetDialog : public QDialog
{
protected:
	QsciScintilla* m_edit = nullptr;

public:
	SourceCodeDataWidgetDialog(QWidget* parent, const std::string& text, bool readOnly)
		: QDialog(parent)
	{
		QVBoxLayout* mainLayout = new QVBoxLayout();

		m_edit = new QsciScintilla(this);
		auto lexer = new QsciLexerCPP(this);
		m_edit->setLexer(lexer);
		m_edit->setEnabled(!readOnly);
		m_edit->setMarginWidth(0, "999");
		m_edit->setAutoCompletionSource(QsciScintilla::AcsAll);
		m_edit->setFolding(QsciScintilla::PlainFoldStyle);
		m_edit->setIndentationWidth(4);
		m_edit->setTabWidth(4);
		m_edit->setIndentationsUseTabs(true);
		m_edit->setAutoIndent(true);
		m_edit->setBackspaceUnindents(true);
		m_edit->setBraceMatching(QsciScintilla::StrictBraceMatch);
		m_edit->setText(QString::fromStdString(text));
		mainLayout->addWidget(m_edit);

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

		setWindowTitle(tr("Source code edit") + (readOnly ? tr(" (read-only)") : ""));

		connect(this, &QDialog::finished, this, &SourceCodeDataWidgetDialog::onFinished);

		QSettings settings("Christophe Guebert", "Panda");
		QSize s = settings.value("SourceCodeDataWidgetDialogSize", QSize(500, 600)).toSize();
		resize(s);
	}

	void onFinished()
	{
		QSettings settings("Christophe Guebert", "Panda");
		settings.setValue("SourceCodeDataWidgetDialogSize", size());
	}

	std::string getText()
	{
		return m_edit->text().toStdString();
	}

	QSize sizeHint() const
	{
		return QSize(500, 400);
	}
};

class SourceCodeDataWidgetContainer : public BaseOpenDialogObject
{
protected:
	typedef std::string value_type;
	QWidget* container;
	QLabel* label;
	std::string theText;
	bool isReadOnly;

public:
	SourceCodeDataWidgetContainer() : container(nullptr), label(nullptr) {}

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
		theText = v;
		label->setText(QString::fromStdString(v));
	}
	void writeToData(value_type& v)
	{
		v = theText;
	}
	virtual void onShowDialog()
	{
		SourceCodeDataWidgetDialog dlg(container, theText, isReadOnly);
		if (dlg.exec() == QDialog::Accepted)
		{
			readFromData(dlg.getText());
			emit editingFinished();
		}
	}
	void updatePreview()
	{
	}
};

//****************************************************************************//

RegisterWidget<SimpleDataWidget<std::string, SourceCodeDataWidgetContainer> > DWClass_source_code("code");
