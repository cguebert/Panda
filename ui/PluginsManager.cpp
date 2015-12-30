#include <ui/PluginsManager.h>

#include <panda/ObjectFactory.h>
#include <panda/helper/system/FileRepository.h>

#include <iostream>

PluginsManager* PluginsManager::getInstance()
{
	static PluginsManager pluginsManager;
	return &pluginsManager;
}

void PluginsManager::loadPlugins()
{
	auto factory = panda::ObjectFactory::getInstance();
	factory->moduleLoaded(); // Register core modules

	const std::string modulesDir = "modules";
	auto modules = panda::helper::system::DataRepository.enumerateFilesInDir(modulesDir);
	for(const auto& moduleName : modules)
	{
		if(QLibrary::isLibrary(QString::fromStdString(moduleName)))
		{
			auto absolutePath = panda::helper::system::DataRepository.findFile(modulesDir + "/" + moduleName);
			LibraryPtr library = LibraryPtr(new QLibrary(QString::fromStdString(absolutePath)));

			if(library->load())
			{
				m_plugins.push_back(library);
				factory->moduleLoaded();
			}
			else
				std::cerr << "Could not load library " << moduleName << std::endl;
		}
	}

	factory->allObjectsRegistered();
}

