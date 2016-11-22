#include <panda/VisualizersManager.h>

#include <panda/data/DataFactory.h>
#include <panda/document/PandaDocument.h>
#include <panda/document/Serialization.h>
#include <panda/helper/Exception.h>
#include <panda/XmlDocument.h>

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace
{

	std::pair<bool, panda::VisualizersManager::VisualizerInformation> getInformation(const std::string& path)
	{
		panda::XmlDocument doc;
		if (!doc.loadFromFile(path))
			return { false, {} };

		const auto root = doc.root();
		const auto dataAtt = root.firstChild("VisualizerData").attribute("type");
		if(!dataAtt)
			return { false, {} };

		panda::VisualizersManager::VisualizerInformation info;
		info.dataType = panda::DataFactory::nameToType(dataAtt.toString());
		info.path = path;
		info.name = fs::path { path }.stem().string();

		return { true, std::move(info) };
	}

}

namespace panda
{

	VisualizersManager::VisualizersManager()
	{
	}

	VisualizersManager& VisualizersManager::instance()
	{
		static VisualizersManager groupManager;
		return groupManager;
	}

	void VisualizersManager::createList(const std::string& dirPath)
	{
		auto& vm = instance();
		vm.m_dirPath = dirPath;
		
		auto& list = vm.m_list;
		list.clear();

		fs::path dir(dirPath);
		if (!dir.is_absolute())
			dir = fs::absolute(dir);
		if (exists(dir) && is_directory(dir))
		{
			for (auto& x : fs::recursive_directory_iterator(dir))
			{
				const auto infoPair = getInformation(x.path().string());
				if (infoPair.first)
					list.push_back(infoPair.second);
			}
		}
	}

	std::unique_ptr<panda::PandaDocument> VisualizersManager::createVisualizer(const std::string& name, panda::gui::BaseGUI& gui)
	{
		const auto& list = instance().m_list;
		auto it = std::find_if(list.begin(), list.end(), [&name](const VisualizerInformation& info) {
			return info.name == name;
		});
		if (it == list.end())
			return nullptr;

		return serialization::readFile(it->path, gui);
	}

	const VisualizersManager::Visualizers& VisualizersManager::visualizers()
	{
		return instance().m_list;
	}

	VisualizersManager::Visualizers VisualizersManager::visualizers(int dataType)
	{
		const auto& list = instance().m_list;
		Visualizers result;
		std::copy_if(list.begin(), list.end(), std::back_inserter(result), [dataType](const VisualizerInformation& info) {
			return info.dataType == dataType;
		});
		return result;
	}

}
