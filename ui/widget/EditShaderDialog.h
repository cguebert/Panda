#ifndef EDITSHADERDIALOG_H
#define EDITSHADERDIALOG_H

#include <panda/types/Shader.h>

#include <ui/widget/DataWidget.h>

#include <QDialog>

#include <map>

class QTabWidget;
class QTextEdit;
class QScrollArea;

class EditShaderDialog : public QDialog
{
	Q_OBJECT
public:
	explicit EditShaderDialog(BaseDataWidget* parent, bool readOnly, QString name);

	void readFromData(const panda::types::Shader& v);
	void writeToData(panda::types::Shader& v);

public slots:

protected:
	void updateValuesTab(const panda::types::Shader::ValuesVector& values);

	bool m_readOnly;
	QTabWidget* m_tabWidget;
	panda::types::Shader::ShaderType m_flags;
	QScrollArea* m_valuesArea;

	typedef std::shared_ptr<BaseDataWidget> DataWidgetPtr;
	std::vector<DataWidgetPtr> m_dataWidgets;

	struct ShaderSourceItem
	{
	//	QWidget* sourceWidget;
		QTextEdit* sourceEdit;

	};
	std::map<panda::types::Shader::ShaderType, ShaderSourceItem> m_sourceWidgets;
};

#endif // EDITSHADERDIALOG_H
