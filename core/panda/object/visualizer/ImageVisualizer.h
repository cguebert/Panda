#pragma once

#include <panda/object/visualizer/Visualizer.h>

namespace panda
{

class PANDA_CORE_API ImageVisualizer : public Visualizer
{
public:
	PANDA_CLASS(ImageVisualizer, Visualizer)

	ImageVisualizer(PandaDocument* doc);

	void update() override;

	unsigned int visualizerTextureId() const override;

protected:
	Data<types::ImageWrapper> m_input;
};

inline unsigned int ImageVisualizer::visualizerTextureId() const
{ return m_input.getValue().getTextureId(); }

} // namespace panda
