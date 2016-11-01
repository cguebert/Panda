#pragma once

#include <panda/object/PandaObject.h>
#include <panda/types/Point.h>
#include <panda/types/ImageWrapper.h>

namespace panda
{

class PANDA_CORE_API Visualizer : public PandaObject
{
public:
	PANDA_CLASS(Visualizer, PandaObject)

	Visualizer(PandaDocument *doc);

	float aspectRatio() const;
	types::Point sizeHint() const;
	const types::ImageWrapper& visualizerImage() const;

protected:
	Data<float> m_aspectRatio;
	Data<types::Point> m_sizeHint;
	Data<types::ImageWrapper> m_visualizerImage;
};

inline float Visualizer::aspectRatio() const
{ return m_aspectRatio.getValue(); }

inline types::Point Visualizer::sizeHint() const
{ return m_sizeHint.getValue(); }

inline const types::ImageWrapper& Visualizer::visualizerImage() const
{ return m_visualizerImage.getValue(); }

} // namespace panda
