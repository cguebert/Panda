#pragma once

#include <panda/object/Visualizer.h>

namespace panda
{

class PANDA_CORE_API ImageVisualizer : public Visualizer
{
public:
	PANDA_CLASS(ImageVisualizer, Visualizer)

	ImageVisualizer(PandaDocument* doc);

	void update();

	const types::ImageWrapper& visualizerImage() const override;

protected:
	Data<types::ImageWrapper> m_input;
};

inline const types::ImageWrapper& ImageVisualizer::visualizerImage() const
{ return m_input.getValue(); }

} // namespace panda
