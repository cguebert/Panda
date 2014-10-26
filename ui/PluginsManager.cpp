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
	panda::ObjectFactory::getInstance()->moduleLoaded(); // Register core modules
#ifdef NDEBUG
	const QString modulesDir = "modules";
#else
	const QString modulesDir = "modules_d";
#endif
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
				panda::ObjectFactory::getInstance()->moduleLoaded();
			}
			else
				std::cerr << "Could not load library " << moduleName.toStdString() << std::endl;
		}
	}
}

