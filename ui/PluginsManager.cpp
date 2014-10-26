#include <ui/PluginsManager.h>
#include <panda/helper/system/FileRepository.h>

#include <iostream>

PluginsManager* PluginsManager::getInstance()
{
	static PluginsManager pluginsManager;
	return &pluginsManager;
}

void PluginsManager::loadPlugins()
{
	QStringList modules = panda::helper::system::DataRepository.enumerateFilesInDir("modules");
	for(const QString& moduleName : modules)
	{
		if(QLibrary::isLibrary(moduleName))
		{
			QString absolutePath = panda::helper::system::DataRepository.findFile("modules/" + moduleName);
			LibraryPtr library = LibraryPtr(new QLibrary(absolutePath));

			if(library->load())
				m_plugins.push_back(library);
			else
				std::cerr << "Could not load library " << moduleName.toStdString() << std::endl;
		}
	}
}

