#ifndef EDITSHADERDIALOG_H
#define EDITSHADERDIALOG_H

#include <panda/types/Shader.h>
#include <panda/graphics/ShaderProgram.h>

#include <ui/widget/DataWidget.h>

#include <QDialog>

#include <map>

class QLabel;
class QTabWidget;
class QScrollArea;
class QsciScintilla;
class QsciLexerGLSL;

class EditShaderDialog : public QDialog
{
	Q_OBJECT
public:
	explicit EditShaderDialog(BaseDataWidget* parent, bool readOnly, QString name);

	void readFromData(const panda::types::Shader& v);
	void writeToData(panda::types::Shader& v);

protected:
	void updateValuesTab(const panda::types::Shader::ValuesVector& values);
	void compileShaders();
	void tabChanged(int index);
	void onFinished();
	void addShader();

	void createTab(int type);

	bool m_readOnly;
	QLabel* m_errorLabel;
	QTabWidget* m_tabWidget;
	QScrollArea* m_valuesArea;
	QsciLexerGLSL* m_lexer;

	typedef std::shared_ptr<BaseDataWidget> DataWidgetPtr;
	std::vector<DataWidgetPtr> m_dataWidgets;

	struct ShaderSourceItem
	{
		QsciScintilla* sourceEdit = nullptr;
		std::string errorText;
		int tabIndex = 0;
		int shaderTypeIndex = 0;
	};
	std::map<panda::types::Shader::ShaderType, ShaderSourceItem> m_sourceWidgets;

	panda::graphics::ShaderProgram m_testProgram;
};

#endif // EDITSHADERDIALOG_H
