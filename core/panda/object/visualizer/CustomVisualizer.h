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

	void setDocument(std::unique_ptr<VisualizerDocument> document);

	void update() override;

	unsigned int visualizerTextureId() const override;

protected:
	std::unique_ptr<VisualizerDocument> m_visualizerDocument;
	std::shared_ptr<BaseData> m_inputData;
};

} // namespace panda
