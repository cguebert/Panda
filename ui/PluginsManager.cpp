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

	const QString modulesDir = "modules";
	QStringList modules = panda::helper::system::DataRepository.enumerateFilesInDir(modulesDir);
	for(const QString& moduleName : modules)
	{
		if(QLibrary::isLibrary(moduleName))
		{
			QString absolutePath = panda::helper::system::DataRepository.findFile(modulesDir + "/" + moduleName);
			LibraryPtr library = LibraryPtr(new QLibrary(absolutePath));

			if(library->load())
			{
				m_plugins.push_back(library);
				factory->moduleLoaded();
			}
			else
				std::cerr << "Could not load library " << moduleName.toStdString() << std::endl;
		}
	}

	factory->allObjectsRegistered();
}

