#pragma once

#include <panda/messaging.h>
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
	virtual const types::ImageWrapper& visualizerImage() const;

	void setDirtyValue(const DataNode* caller) override;

	Data<types::Point> visualizerSize;

	BaseData* visualizedData() const;

	msg::Signal<void()> dirtyVisualization;

protected:
	BaseData* m_visualizedData = nullptr;
	Data<float> m_aspectRatio;
	Data<types::ImageWrapper> m_visualizerImage;
};

inline float Visualizer::aspectRatio() const
{ return m_aspectRatio.getValue(); }

inline const types::ImageWrapper& Visualizer::visualizerImage() const
{ return m_visualizerImage.getValue(); }

inline BaseData* Visualizer::visualizedData() const
{ return m_visualizedData; }

} // namespace panda
