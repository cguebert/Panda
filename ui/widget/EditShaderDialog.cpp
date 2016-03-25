#include <ui/widget/DataWidgetFactory.h>
#include <ui/widget/EditShaderDialog.h>
#include <ui/widget/ListDataWidgetDialog.h>
#include <ui/widget/OpenDialogDataWidget.h>

#include <Qsci/qsciscintilla.h>
#include <Qsci/qscilexerglsl.h>
#include <QtWidgets>

using panda::types::Shader;
using panda::graphics::ShaderProgram;

namespace
{

panda::graphics::ShaderType convert(panda::types::Shader::ShaderType type)
{
	return static_cast<panda::graphics::ShaderType>(static_cast<char>(type));
}

struct ShaderTypesInfo
{
	QStringList typesNames;
	std::vector<Shader::ShaderType> typesValues;
	int nbTypes = 0;
};

static const ShaderTypesInfo& getShaderTypesInfo()
{
	static ShaderTypesInfo info;
	if (!info.nbTypes)
	{
		info.typesNames << "Vertex" << "Fragment" << "Geometry"
			<< "TessellationControl" << "TessellationEvaluation" << "Compute";
		using ShaderType = panda::types::Shader::ShaderType;
		info.typesValues = { ShaderType::Vertex, ShaderType::Fragment, ShaderType::Geometry, 
			ShaderType::TessellationControl, ShaderType::TessellationEvaluation, ShaderType::Compute};
		info.nbTypes = info.typesNames.size();
	}

	return info;
}

int getShaderIndex(panda::types::Shader::ShaderType type)
{
	const auto& info = getShaderTypesInfo();
	for (int i = 0; i < info.nbTypes; ++i)
	{
		if (info.typesValues[i] == type)
			return i;
	}

	return 0;
}

}

EditShaderDialog::EditShaderDialog(BaseDataWidget* parent, bool readOnly, QString name)
	: QDialog(parent)
	, m_readOnly(readOnly)
{
	QVBoxLayout* mainLayout = new QVBoxLayout();

	m_tabWidget = new QTabWidget;
	m_tabWidget->setMinimumWidth(400);
	connect(m_tabWidget, &QTabWidget::currentChanged, this, &EditShaderDialog::tabChanged);
	mainLayout->addWidget(m_tabWidget);

	m_errorLabel = new QLabel;
	m_errorLabel->setStyleSheet("QLabel { color : red; }");
	m_errorLabel->hide();
	mainLayout->addWidget(m_errorLabel);
	
	QHBoxLayout* buttonsLayout = new QHBoxLayout;
	if (!m_readOnly)
	{
		QPushButton* compileButton = new QPushButton(tr("Compile"), this);
		compileButton->setShortcut(tr("Ctrl+S"));
		connect(compileButton, &QPushButton::clicked, this, &EditShaderDialog::compileShaders);
		buttonsLayout->addWidget(compileButton);
		buttonsLayout->addStretch();

		QPushButton* addShaderButton = new QPushButton(tr("Add shader"), this);
		connect(addShaderButton, &QPushButton::clicked, this, &EditShaderDialog::addShader);
		buttonsLayout->addWidget(addShaderButton);
	}

	QPushButton* okButton = new QPushButton(tr("Ok"), this);
	okButton->setDefault(true);
	connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
	buttonsLayout->addStretch();
	buttonsLayout->addWidget(okButton);

	QPushButton* cancelButton = new QPushButton(tr("Cancel"), this);
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	buttonsLayout->addWidget(cancelButton);
	mainLayout->addLayout(buttonsLayout);

	setLayout(mainLayout);
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	setWindowTitle(name + (readOnly ? tr(" (read-only)") : ""));

	m_lexer = new QsciLexerGLSL(this);

	// Parse the widget parameters (which shaders types to accept)
	const auto& info = getShaderTypesInfo();
	QString params = parent->getParameters();
	if(params.isEmpty())
		params = "Vertex:Fragment"; // By default, create vertex & fragment shaders
	QStringList typesList = params.split(";");
	for (const QString& type : typesList)
	{
		for (int i = 0; i < info.nbTypes; ++i)
		{
			if (!type.compare(info.typesNames[i], Qt::CaseInsensitive))
			{
				createTab(i);
				break;
			}
		}
	}

	m_valuesArea = new QScrollArea();
	m_valuesArea->setFrameShape(QFrame::NoFrame);
	m_valuesArea->setWidgetResizable(true);
	m_tabWidget->addTab(m_valuesArea, "Uniforms");

	connect(this, &QDialog::finished, this, &EditShaderDialog::onFinished);

	QSettings settings("Christophe Guebert", "Panda");
	QSize s = settings.value("EditShaderDialogSize", QSize(500, 600)).toSize();
	resize(s);
}

void EditShaderDialog::updateValuesTab(const Shader::ValuesVector& values)
{
	QWidget* dummyContainer = new QWidget();
	QFormLayout* valuesLayout = new QFormLayout();
	dummyContainer->setLayout(valuesLayout);
	m_valuesArea->setWidget(dummyContainer);

	m_dataWidgets.clear();

	for(auto value : values)
	{
		DataWidgetPtr dataWidget = DataWidgetPtr(DataWidgetFactory::getInstance()->create(
			dummyContainer, value->getValue(), value->dataTrait()->fullTypeId(),
			"", QString::fromStdString(value->getName()), "") );

		if (dataWidget)
		{
			m_dataWidgets.push_back(dataWidget);
			QWidget* widget = dataWidget->createWidgets(m_readOnly);
			valuesLayout->addRow(QString::fromStdString(value->getName()), widget);
		}
	}
}

void EditShaderDialog::readFromData(const Shader& shader)
{
	const auto& sources = shader.getSources();
	for(const auto& source : sources)
	{
		if (!m_sourceWidgets.count(source.type))
			createTab(getShaderIndex(source.type));

		m_sourceWidgets.at(source.type).sourceEdit->setText(QString::fromStdString(source.sourceCode));
	}

	auto values = shader.getValues();
	updateValuesTab(values);
}

void EditShaderDialog::writeToData(Shader& shader)
{
	for(const auto& it : m_sourceWidgets)
		shader.setSource(it.first, it.second.sourceEdit->text().toStdString());
}

void EditShaderDialog::compileShaders()
{
	const auto& info = getShaderTypesInfo();
	m_testProgram.clear();
	int currentTab = m_tabWidget->currentIndex();
	bool error = false;
	for (auto& it : m_sourceWidgets)
	{
		auto type = convert(it.first);
		ShaderSourceItem& item = it.second;
		std::string errorText;
		std::string sourceCode = item.sourceEdit->text().toStdString();
		auto id = panda::graphics::ShaderProgram::compileShader(type, sourceCode, &errorText);
		item.errorText = errorText;
		m_tabWidget->setTabText(item.tabIndex, (id ? "" : "! ") + info.typesNames[item.shaderTypeIndex]);
		if (!id)
			error = true;
		else
			m_testProgram.addShader(id);

		if (item.tabIndex == currentTab)
		{
			if(id)
				m_errorLabel->hide();
			else
			{
				m_errorLabel->setText(QString::fromStdString(errorText));
				m_errorLabel->show();
			}
		}
	}

	if (error)
		return;

	std::string errorText;
	if (!m_testProgram.link(&errorText))
	{
		m_errorLabel->setText(QString::fromStdString(errorText));
		m_errorLabel->show();
	}
}

void EditShaderDialog::tabChanged(int index)
{
	bool error = false;

	for (const auto& it : m_sourceWidgets)
	{
		if (it.second.tabIndex != index)
			continue;

		if (!it.second.errorText.empty())
		{
			m_errorLabel->setText(QString::fromStdString(it.second.errorText));
			m_errorLabel->show();
			error = true;
			break;
		}
	}

	if(!error)
		m_errorLabel->hide();
}

void EditShaderDialog::onFinished()
{
	QSettings settings("Christophe Guebert", "Panda");
	settings.setValue("EditShaderDialogSize", size());
}

void EditShaderDialog::addShader()
{
	const auto& info = getShaderTypesInfo();
	QStringList availableTypes;
	for (int i = 0; i < info.nbTypes; ++i)
	{
		if (!m_sourceWidgets.count(info.typesValues[i]))
			availableTypes.push_back(info.typesNames[i]);
	}

	if (availableTypes.empty())
		return;

	bool ok = false;
	auto selected = QInputDialog::getItem(this, "Add shader", "Shader type:", availableTypes, 0, false, &ok);

	if (!ok)
		return;

	for (int i = 0; i < info.nbTypes; ++i)
	{
		if (info.typesNames[i] == selected)
		{
			createTab(i);
			break;
		}
	}
}

void EditShaderDialog::createTab(int typeIndex)
{
	const auto& info = getShaderTypesInfo();

	ShaderSourceItem item;
	item.shaderTypeIndex = typeIndex;
	auto edit = new QsciScintilla(this);
	edit->setLexer(m_lexer);
	edit->setEnabled(!m_readOnly);
	edit->setMarginWidth(0, "999");
	edit->setAutoCompletionSource(QsciScintilla::AcsAll);
	edit->setFolding(QsciScintilla::PlainFoldStyle);
	edit->setIndentationWidth(4);
	edit->setTabWidth(4);
	edit->setIndentationsUseTabs(true);
	edit->setAutoIndent(true);
	edit->setBackspaceUnindents(true);
	edit->setEolMode(QsciScintilla::EolUnix);
	edit->setBraceMatching(QsciScintilla::StrictBraceMatch);
	item.sourceEdit = edit;

	// Find where to put this tab
	int tabIndex = 0;
	for (int i = 0; i < typeIndex; ++i)
	{
		if (m_sourceWidgets.count(info.typesValues[i]))
			++tabIndex;
	}

	item.tabIndex = m_tabWidget->insertTab(tabIndex, item.sourceEdit, info.typesNames[typeIndex]);
	m_sourceWidgets[info.typesValues[typeIndex]] = item;

	// Update the tab index of the other shaders
	for (int i = typeIndex + 1; i < info.nbTypes; ++i)
	{
		auto t = info.typesValues[i];
		if (m_sourceWidgets.count(t))
			++m_sourceWidgets[t].tabIndex;
	}
}

//****************************************************************************//

RegisterWidget<OpenDialogDataWidget<Shader, EditShaderDialog> > DWClass_shader("default");
RegisterWidget<OpenDialogDataWidget<std::vector<Shader>, ListDataWidgetDialog<std::vector<Shader> > > > DWClass_shaders_list("generic");
