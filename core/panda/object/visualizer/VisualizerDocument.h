#pragma once

#include <panda/document/RenderedDocument.h>
#include <panda/document/DocumentDatas.h>

namespace panda
{

class PANDA_CORE_API VisualizerDocument : public RenderedDocument
{
public:
	PANDA_CLASS(VisualizerDocument, RenderedDocument)

	VisualizerDocument(gui::BaseGUI& gui);

	void setVisualizerType(int type);
	int visualizerType() const;

	DocumentDatas& documentDatas();

	void save(XmlElement& elem, const std::vector<PandaObject*> *selected = nullptr) override;
	void load(const XmlElement& elem) override;

protected:
	using DataPtr = std::shared_ptr<BaseData>;
	using CreatedDataList = std::vector<DataPtr>;

	int m_visualizerType = 0;
	DocumentDatas m_documentDatas;
};

inline int VisualizerDocument::visualizerType() const
{ return m_visualizerType; }

inline DocumentDatas& VisualizerDocument::documentDatas()
{ return m_documentDatas; }

} // namespace panda
