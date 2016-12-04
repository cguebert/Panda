#pragma once

#include <panda/object/visualizer/Visualizer.h>

namespace panda
{

class VisualizerDocument;

class PANDA_CORE_API CustomVisualizer : public Visualizer
{
public:
	PANDA_CLASS(CustomVisualizer, Visualizer)

	CustomVisualizer(PandaDocument* doc);

	void setDocumentPath(const std::string& path);

	void update() override;

	unsigned int visualizerTextureId() const override;

protected:
	void load(const XmlElement& elem) override;

	std::unique_ptr<VisualizerDocument> m_visualizerDocument;
	std::shared_ptr<BaseData> m_inputData;

	BaseData* m_docInputData = nullptr;
	bool m_initialized = false;
	Data<std::string> m_docPath;
};

} // namespace panda
