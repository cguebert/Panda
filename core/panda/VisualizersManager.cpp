#include <panda/VisualizersManager.h>

#include <panda/data/DataFactory.h>
#include <panda/document/PandaDocument.h>
#include <panda/document/Serialization.h>
#include <panda/helper/Exception.h>
#include <panda/helper/system/FileRepository.h>
#include <panda/XmlDocument.h>

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace
{

	std::pair<bool, panda::VisualizersManager::VisualizerInformation> getInformation(const std::string& dirPath, const std::string& path)
	{
		const auto filePath = (fs::path(dirPath) / path).generic_string();
		const auto realPath = panda::helper::system::DataRepository.findFile(filePath);
		panda::XmlDocument doc;
		if (!doc.loadFromFile(realPath))
			return { false, {} };

		const auto root = doc.root();
		const auto dataAtt = root.firstChild("VisualizerData").attribute("type");
		if(!dataAtt)
			return { false, {} };

		panda::VisualizersManager::VisualizerInformation info;
		info.dataType = panda::DataFactory::nameToType(dataAtt.toString());
		info.path = filePath;
		info.name = fs::path { path }.stem().string();

		return { true, std::move(info) };
	}

}

namespace panda
{

	const std::string VisualizersManager::m_dirPath = "visualizers";

	VisualizersManager::VisualizersManager()
	{
	}

	VisualizersManager& VisualizersManager::instance()
	{
		static VisualizersManager groupManager;
		return groupManager;
	}

	void VisualizersManager::createList()
	{
		auto& vm = instance();
		
		auto& list = vm.m_list;
		list.clear();

		const auto paths = panda::helper::system::DataRepository.enumerateFilesInDir(m_dirPath);
		for (auto&& path : paths)
		{
			const auto infoPair = getInformation(m_dirPath, path);
			if (infoPair.first)
				list.push_back(infoPair.second);
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

		const auto filePath = fs::path(m_dirPath) / it->path;
		const auto realPath = panda::helper::system::DataRepository.findFile(filePath.string());
		return serialization::readFile(realPath, gui);
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
