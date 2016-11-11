#pragma once

#include <panda/document/RenderedDocument.h>

namespace panda
{

class PANDA_CORE_API VisualizerDocument : public RenderedDocument
{
public:
	PANDA_CLASS(VisualizerDocument, RenderedDocument)

	VisualizerDocument(gui::BaseGUI& gui);

	void setVisualizerType(int type);
	int visualizerType() const;

	void save(XmlElement& elem, const std::vector<PandaObject*> *selected = nullptr) override;
	void load(const XmlElement& elem) override;

protected:
	using DataPtr = std::shared_ptr<BaseData>;
	using CreatedDataList = std::vector<DataPtr>;

	int m_visualizerType = 0;
};

inline int VisualizerDocument::visualizerType() const
{ return m_visualizerType; }

} // namespace panda