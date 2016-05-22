#pragma once

#include <panda/object/ObjectAddons.h>

namespace panda
{
	class BaseData;
}

std::string getDataLabel(panda::BaseData* data);
void setDataLabel(panda::BaseData* data, const std::string& label);

class DataLabelAddon : public panda::BaseObjectAddon
{
public:
	struct DataLabel
	{
		panda::BaseData* data;
		std::string label;
	};

	DataLabelAddon(panda::PandaObject& object);

	void save(panda::XmlElement& elem) override;
	void load(panda::XmlElement& elem) override;

	std::string getLabel(panda::BaseData* data) const;
	void setLabel(panda::BaseData* data, const std::string& label);

private:
	std::vector<DataLabel> m_dataLabels;
};