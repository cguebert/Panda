#include <panda/command/DocumentDatasCommand.h>
#include <panda/document/DocumentDatas.h>
#include <panda/document/PandaDocument.h>

#include <algorithm>

namespace panda
{

AddDataToDocumentDatasCommand::AddDataToDocumentDatasCommand(DocumentDatas& documentDatas, std::shared_ptr<BaseData> data, bool isInput, bool isOutput)
	: m_documentDatas(documentDatas)
	, m_data(data)
	, m_input(isInput)
	, m_output(isOutput)
{
	setText("add group data");
}

void AddDataToDocumentDatasCommand::redo()
{
	auto obj = m_documentDatas.parent();
	if (m_input)
		obj->addInput(*m_data);
	if (m_output)
		obj->addOutput(*m_data);

	m_documentDatas.add(m_data);
	obj->addData(m_data.get());
}

void AddDataToDocumentDatasCommand::undo()
{
	auto obj = m_documentDatas.parent();
	if (m_input)
		obj->removeInput(*m_data);
	if (m_output)
		obj->removeOutput(*m_data);

	m_documentDatas.remove(m_data);
	obj->removeData(m_data.get());
}

//****************************************************************************//

RemoveDataFromDocumentDatasCommand::RemoveDataFromDocumentDatasCommand(DocumentDatas& documentDatas, BaseData* data)
	: m_documentDatas(documentDatas)
{
	setText("remove group data");

	const auto& datas = m_documentDatas.get();
	auto dIt = std::find_if(datas.begin(), datas.end(), [data](const std::shared_ptr<BaseData>& dataSPtr) {
		return dataSPtr.get() == data;
	});
	m_data = *dIt;
	m_documentDataIndex = std::distance(datas.begin(), dIt);

	const auto& objDatas = m_documentDatas.parent()->getDatas();
	auto oIt = std::find_if(objDatas.begin(), objDatas.end(), [data](const BaseData* dataPtr) {
		return dataPtr == data;
	});
	m_dataIndex = std::distance(objDatas.begin(), oIt);

	m_input = data->isInput();
	m_output = data->isOutput();
}

void RemoveDataFromDocumentDatasCommand::redo()
{
	auto obj = m_documentDatas.parent();
	if (m_input)
		obj->removeInput(*m_data);
	if (m_output)
		obj->removeOutput(*m_data);

	m_documentDatas.remove(m_data);
	obj->removeData(m_data.get());
}

void RemoveDataFromDocumentDatasCommand::undo()
{
	auto obj = m_documentDatas.parent();
	if (m_input)
		obj->addInput(*m_data);
	if (m_output)
		obj->addOutput(*m_data);

	m_documentDatas.add(m_data, m_documentDataIndex);
	obj->addData(m_data.get(), m_dataIndex);
}

} // namespace panda
