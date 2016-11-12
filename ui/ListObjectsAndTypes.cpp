#include <QTemporaryFile>
#include <QTextStream>
#include <QDir>

#include <ui/ListObjectsAndTypes.h>
#include <ui/widget/DataWidgetFactory.h>
#include <panda/object/ObjectFactory.h>
#include <panda/data/DataFactory.h>
#include <panda/types/TypeConverter.h>
#include <panda/object/PandaObject.h>

using namespace panda;
using namespace panda::types;

bool dataTypeIdLessThan(const std::shared_ptr<DataFactory::DataEntry>& e1, const std::shared_ptr<DataFactory::DataEntry>& e2)
{
	return e1->fullType < e2->fullType;
}

bool objectsMenuLessThan(const ObjectFactory::ClassEntry& e1, const ObjectFactory::ClassEntry& e2)
{
	return e1.menuDisplay < e2.menuDisplay;
}

QString toHtml(std::string text)
{
	QString result = QString::fromStdString(text);
	result.replace("<", "&lt;");
	result.replace(">", "&gt;");
	return result;
}

struct EnrichedDataEntry
{
	EnrichedDataEntry()
		: fullType(0)
		, trait(nullptr)
	{}
	EnrichedDataEntry(const DataFactory::DataEntry& e)
		: typeName(e.typeName)
		, className(e.className)
		, description(e.typeName)
		, fullType(e.fullType)
		, trait(nullptr)
	{}

	std::string typeName;
	std::string className;
	std::string description;
	int fullType;
	const AbstractDataTrait* trait;
};

bool canConvert(const AbstractDataTrait* from, const AbstractDataTrait* to)
{
	if(from->isAnimation() != to->isAnimation())
		return false;

	int fromFull = from->fullTypeId(), fromValue = from->valueTypeId();
	int toFull = to->fullTypeId(), toValue = to->valueTypeId();

	return (fromValue == toValue)
		|| TypeConverter::canConvert(fromFull, toFull)
		|| TypeConverter::canConvert(fromFull, toValue)
		|| TypeConverter::canConvert(fromValue, toFull)
		|| TypeConverter::canConvert(fromValue, toValue);
}

QString createObjectsAndTypesPage(PandaDocument* document)
{
	static QTemporaryFile file(QDir::tempPath() + "/pandaRef_XXXXXX.html");
	file.open();
	file.resize(0);

	QTextStream out(&file);
	out << "<!DOCTYPE html>\n"
		   "<html lang=\"en\">\n"
		   " <head>\n"
		   "  <meta charset=\"utf-8\">\n"
		   "  <title>List of available objects and types for Panda</title>\n"
		   " </head>\n"
		   " <body>\n";

	out << "  <h1>Types</h1>\n";

	auto dataEntriesList = DataFactory::entries();
	std::sort(dataEntriesList.begin(), dataEntriesList.end(), dataTypeIdLessThan);

	std::vector<EnrichedDataEntry> dataEntries;
	// Copy entries and get additional info
	for(auto d : dataEntriesList)
	{
		EnrichedDataEntry e = *d;
		auto tmpData = DataFactory::create(d->fullType, "", "", nullptr);
		if(tmpData)
		{
			e.description = tmpData->getDescription();
			e.trait = tmpData->getDataTrait();
		}

		dataEntries.push_back(e);
	}

	for(const auto& d : dataEntries)
	{
		out << "   <h2>" << QString::fromStdString(d.description) << "</h2>\n"
			<< "    <p>Type: " << toHtml(d.className) << "</p>\n"
			<< "    <p>Id: " << d.fullType << "</p>\n";

		bool hasConversions = false;
		for(const auto& d2 : dataEntries)
		{
			if(d.fullType == d2.fullType)
				continue;

			if(!d.trait || !d2.trait)
				continue;

			if(canConvert(d.trait, d2.trait))
			{
				if(!hasConversions)
				{
					out << "    <p>Can be converted to: ";
					hasConversions = true;
				}
				else
					out << ", ";
				out << QString::fromStdString(d2.description);
			}
		}
		if(!hasConversions)
			out << "    <p>Has no known conversion";
		out << "</p>\n";

		auto widgets = DataWidgetFactory::getInstance()->getWidgetNames(d.fullType);
		if(widgets.empty())
			out << "    <p>No widgets for this type</p>\n";
		else
		{
			out << "    <p>Widgets available: ";
			bool first = true;
			for(const auto& w : widgets)
			{
				if(first)
					first = false;
				else
					out << ", ";
				out << w;
			}
			out << "</p>\n";
		}
	}

	out << "  <h1>Objects</h1>\n";

	std::vector<ObjectFactory::ClassEntry> objectsEntries;
	for(const auto& object : ObjectFactory::registryMap())
		objectsEntries.push_back(object.second);
	std::sort(objectsEntries.begin(), objectsEntries.end(), objectsMenuLessThan);

	for(const auto& o : objectsEntries)
	{
		out << "   <h2>" << QString::fromStdString(o.menuDisplay) << "</h2>\n";
		out << "	<p>In module: " << toHtml(o.moduleName) << "</p>\n";
		out << "    <p>Type: " << toHtml(o.className) << "</p>\n";
		out << "    <p>" << QString::fromStdString(o.description) << "</p>\n";

		if(o.creator)
		{
			auto object = o.creator->create(document);

			auto datas = object->getDatas();
			if(!datas.empty())
				out << "    <h3>Datas</h3>\n";
			for(auto d : datas)
			{
				out << "     <p>" << QString::fromStdString(d->getName()) << ": "
					<< QString::fromStdString(d->getDescription()) << ". "
					<< QString::fromStdString(d->getHelp()) << "</p>\n";
			}
		}
	}

	out << " </body>\n"
		   "</html>\n";

	return file.fileName();
}
