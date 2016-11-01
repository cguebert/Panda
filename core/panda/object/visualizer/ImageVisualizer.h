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

protected:
	Data<types::ImageWrapper> m_input;
};


} // namespace panda
