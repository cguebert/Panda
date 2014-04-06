#include <ui/widget/DataWidgetFactory.h>
#include <ui/widget/EditShaderDialog.h>
#include <ui/widget/ListDataWidgetDialog.h>
#include <ui/widget/OpenDialogDataWidget.h>

#include <QtWidgets>

using panda::types::Shader;

EditShaderDialog::EditShaderDialog(BaseDataWidget* parent, bool readOnly, QString name)
	: QDialog(parent)
{
	QVBoxLayout* mainLayout = new QVBoxLayout();

	m_tabWidget = new QTabWidget;
	mainLayout->addWidget(m_tabWidget);

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

	// Parse the widget parameters (which shaders types to accept)
	const char* typesNames[] = { "Vertex", "Fragment", "Geometry",
								 "TessellationControl", "TessellationEvaluation", "Compute" };
	QOpenGLShader::ShaderType typesValues[] = { QOpenGLShader::Vertex, QOpenGLShader::Fragment,
												QOpenGLShader::Geometry, QOpenGLShader::TessellationControl,
												QOpenGLShader::TessellationEvaluation, QOpenGLShader::Compute};
	const int nbTypes = sizeof(typesNames) / sizeof(const char*);

	QString params = parent->getParameters();
	QStringList typesList = params.split(";");
	for(const QString& type : typesList)
	{
		for(int i=0; i<nbTypes; ++i)
		{
			if(!type.compare(typesNames[i], Qt::CaseInsensitive))
			{
				ShaderSourceItem item;
				item.sourceEdit = new QTextEdit;
				item.sourceEdit->setEnabled(!readOnly);
				m_sourceWidgets[typesValues[i]] = item;
				m_tabWidget->addTab(item.sourceEdit, typesNames[i]);
				break;
			}
		}
	}
}

void EditShaderDialog::readFromData(const Shader& shader)
{
	auto sources = shader.getSources();
	for(auto source : sources)
	{
		if(m_sourceWidgets.contains(source.type))
		{
			m_sourceWidgets[source.type].sourceEdit->setPlainText(source.sourceCode);
		}
	}
}

void EditShaderDialog::writeToData(Shader& shader)
{
	for(auto it = m_sourceWidgets.cbegin(), itEnd = m_sourceWidgets.cend(); it != itEnd; ++it)
		shader.addSource(it.key(), it.value().sourceEdit->toPlainText());
}

//***************************************************************//

RegisterWidget<OpenDialogDataWidget<Shader, EditShaderDialog> > DWClass_shader("default");
RegisterWidget<OpenDialogDataWidget<QVector<Shader>, ListDataWidgetDialog<QVector<Shader> > > > DWClass_shaders_list("generic");
