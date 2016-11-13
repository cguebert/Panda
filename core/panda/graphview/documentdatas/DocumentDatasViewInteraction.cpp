#include <panda/graphview/documentdatas/DocumentDatasViewInteraction.h>
#include <panda/graphview/documentdatas/DocumentDatasView.h>

#include <panda/graphview/ViewGUI.h>
#include <panda/graphview/Viewport.h>

#include <panda/TimedFunctions.h>
#include <panda/command/DocumentDatasCommand.h>
#include <panda/command/LinkDatasCommand.h>
#include <panda/document/DocumentDatas.h>
#include <panda/document/PandaDocument.h>

namespace panda
{

namespace graphview
{

	DocumentDatasViewInteraction::DocumentDatasViewInteraction(DocumentDatasView& view, DocumentDatas& documentDatas)
		: ViewInteraction(view)
		, m_documentDatasView(view)
		, m_documentDatas(documentDatas)
	{
	}

	void DocumentDatasViewInteraction::contextMenuEvent(const ContextMenuEvent& event)
	{
		m_contextMenuData = nullptr;

		auto pos = m_view.viewport().toView(event.pos());
		int flags = getContextMenuFlags(pos);

		TimedFunctions::cancelRun(m_hoverTimerId);

		ViewGui::Actions actions;

		if (m_contextMenuData)
		{
			if (m_contextMenuData->isInput())
			{
				if (!m_contextMenuData->getParent())
					actions.emplace_back("Add input data", 
											"Add an new input data, based on and connected to this data", 
											[this]() { createInputData(); });
			}
			else if (m_contextMenuData->isOutput())
			{
				bool connected = false;
				const auto& outputs = m_contextMenuData->getOutputs();
				if (!outputs.empty())
				{
					for (const auto output : outputs)
					{
						auto data = dynamic_cast<BaseData*>(output);
						if (data && data->getOwner() == m_documentDatas.parent())
						{
							connected = true;
							break;
						}
					}
				}

				if (!connected)
					actions.emplace_back("Add output data", 
											"Add an new output data, based on and connected to this data", 
											[this]() { createOutputData(); });
			}
		}
		else
		{
			for (const auto& dataRect : m_documentDatasView.dataRects())
			{
				if (dataRect.second.contains(pos) && dataRect.first->isDynamicallyCreated())
				{
					m_contextMenuData = dataRect.first;
					if (m_contextMenuData->isInput())
						actions.emplace_back("Remove input data",
												"Disconnect this data and remove it",
												[this]() {
						auto macro = m_view.document()->getUndoStack().beginMacro("remove input data");
						removeData(m_contextMenuData);
					});
					else if (m_contextMenuData->isOutput())
						actions.emplace_back("Remove output data",
												"Disconnect this data and remove it",
												[this]() {
						auto macro = m_view.document()->getUndoStack().beginMacro("remove output data");
						removeData(m_contextMenuData);
					});
					break;
				}
			}
		}

		m_view.gui().contextMenu(event.pos(), flags, actions);
	}

	void DocumentDatasViewInteraction::createInputData()
	{
		auto& undoStack = m_view.document()->getUndoStack();
		auto macro = undoStack.beginMacro("create input data");

		auto data = contextMenuData();
		auto newData = duplicateData(m_documentDatas.parent(), data);
		undoStack.push(std::make_shared<AddDataToDocumentDatasCommand>(m_documentDatas, newData, true, false));
		auto createdData = newData.get();
		createdData->copyValueFrom(data);
		undoStack.push(std::make_shared<LinkDatasCommand>(data, createdData));
	}

	void DocumentDatasViewInteraction::createOutputData()
	{
		auto& undoStack = m_view.document()->getUndoStack();
		auto macro = undoStack.beginMacro("create output data");

		auto data = contextMenuData();
		auto newData = duplicateData(m_documentDatas.parent(), data);
		undoStack.push(std::make_shared<AddDataToDocumentDatasCommand>(m_documentDatas, newData, false, true));
		undoStack.push(std::make_shared<LinkDatasCommand>(newData.get(), data));
	}

	void DocumentDatasViewInteraction::removeData(BaseData* data)
	{
		auto& undoStack = m_view.document()->getUndoStack();

		auto outputs = data->getOutputs();
		for (auto node : outputs)
		{
			auto outData = dynamic_cast<BaseData*>(node);
			if (outData)
				undoStack.push(std::make_shared<LinkDatasCommand>(outData, nullptr));
		}

		undoStack.push(std::make_shared<RemoveDataFromDocumentDatasCommand>(m_documentDatas, data));
		undoStack.push(std::make_shared<LinkDatasCommand>(data, nullptr));
	}

} // namespace graphview

} // namespace panda
