#include <panda/document/DocumentUtils.h>

#include <panda/command/AddObjectCommand.h>
#include <panda/data/DataFactory.h>
#include <panda/document/PandaDocument.h>
#include <panda/document/ObjectsList.h>
#include <panda/object/ObjectFactory.h>

namespace panda 
{

void copyDataToUserValue(const BaseData* data, PandaDocument* document, ObjectsList& objectList)
{
	if(!data)
		return;

	auto entry = DataFactory::entry(data->getDataTrait()->fullTypeId());
	if(!entry)
		return;

	std::string registryName = std::string("panda::GeneratorUser<") + entry->className + ">";
	auto object = ObjectFactory::create(registryName, document);
	document->getUndoStack().push(std::make_shared<panda::AddObjectCommand>(document, objectList, object));
	if(!object)
		return;

	objectList.addObject(object);
	BaseData* inputData = object->getData("input");
	if(inputData)
	{
		inputData->copyValueFrom(data);
		inputData->setWidget(data->getWidget());
		inputData->setWidgetData(data->getWidgetData());
	}

	BaseData* captionData = object->getData("caption");
	if (captionData)
	{
		auto captionTextData = dynamic_cast<Data<std::string>*>(captionData);
		if (captionTextData)
			captionTextData->setValue(data->getName());
	}
}

} // namespace panda


