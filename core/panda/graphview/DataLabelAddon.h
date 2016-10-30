#pragma once

#include <panda/object/ObjectAddons.h>
#include <panda/messaging.h>

namespace panda
{
	class BaseData;

namespace graphview
{

class PANDA_CORE_API DataLabelAddon : public BaseObjectAddon
{
public:
	struct DataLabel
	{
		BaseData* data;
		std::string label;
	};

	DataLabelAddon(PandaObject& object);

	static void setDefinition(ObjectAddonNodeDefinition& nodeDefinition);

	void save(ObjectAddonNode& node) override;
	void load(const ObjectAddonNode& node) override;

	void objectModified() override;

	std::string getLabel(BaseData* data) const;
	void setLabel(BaseData* data, const std::string& label);

	static std::string getDataLabel(BaseData* data);
	static void setDataLabel(BaseData* data, const std::string& label);

private:
	std::vector<DataLabel> m_dataLabels;
	msg::Observer m_observer;
};

} // namespace graphview

} // namespace panda
