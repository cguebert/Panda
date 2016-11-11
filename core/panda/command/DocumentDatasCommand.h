#pragma once

#include <panda/core.h>

#include <panda/UndoStack.h>

namespace panda
{

class BaseData;
class DocumentDatas;
class PandaObject;

class PANDA_CORE_API AddDataToDocumentDatasCommand : public UndoCommand
{
public:
	AddDataToDocumentDatasCommand(DocumentDatas& documentDatas, std::shared_ptr<BaseData> data, bool isInput, bool isOutput);

	virtual void redo();
	virtual void undo();

protected:
	DocumentDatas& m_documentDatas;
	std::shared_ptr<BaseData> m_data;
	bool m_input, m_output;
};

//****************************************************************************//

class PANDA_CORE_API RemoveDataFromDocumentDatasCommand : public UndoCommand
{
public:
	RemoveDataFromDocumentDatasCommand(DocumentDatas& documentDatas, BaseData* data);

	virtual void redo();
	virtual void undo();

protected:
	DocumentDatas& m_documentDatas;
	std::shared_ptr<BaseData> m_data;
	int m_documentDataIndex, m_dataIndex;
	bool m_input, m_output;
};

} // namespace panda
