#ifndef EDITSHADERDIALOG_H
#define EDITSHADERDIALOG_H

#include <panda/types/Shader.h>

#include <ui/widget/DataWidget.h>

#include <QDialog>
#include <QMap>

class QTabWidget;
class QTextEdit;

class EditShaderDialog : public QDialog
{
	Q_OBJECT
public:
	explicit EditShaderDialog(BaseDataWidget* parent, bool readOnly, QString name);

	void readFromData(const panda::types::Shader& v);
	void writeToData(panda::types::Shader& v);

public slots:

protected:
	QTabWidget* m_tabWidget;
	QOpenGLShader::ShaderType m_flags;

	struct ShaderSourceItem
	{
	//	QWidget* sourceWidget;
		QTextEdit* sourceEdit;

	};
	QMap<QOpenGLShader::ShaderType, ShaderSourceItem> m_sourceWidgets;
};

#endif // EDITSHADERDIALOG_H
