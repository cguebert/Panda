#pragma once

#include <panda/object/ObjectAddons.h>
#include <panda/messaging.h>

namespace panda
{
	class BaseData;
}

namespace graphview
{

class PANDA_CORE_API DataLabelAddon : public panda::BaseObjectAddon
{
public:
	struct DataLabel
	{
		panda::BaseData* data;
		std::string label;
	};

	DataLabelAddon(panda::PandaObject& object);

	static void setDefinition(panda::ObjectAddonNodeDefinition& nodeDefinition);

	void save(panda::ObjectAddonNode& node) override;
	void load(const panda::ObjectAddonNode& node) override;

	void objectModified() override;

	std::string getLabel(panda::BaseData* data) const;
	void setLabel(panda::BaseData* data, const std::string& label);

	static std::string getDataLabel(panda::BaseData* data);
	static void setDataLabel(panda::BaseData* data, const std::string& label);

private:
	std::vector<DataLabel> m_dataLabels;
	panda::msg::Observer m_observer;
};

} // namespace graphview
