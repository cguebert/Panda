#include <QCoreApplication>
#include <QLibrary>

#include <ui/PluginsManager.h>

PluginsManager::PluginsManager()
{
	m_pluginsDirPath = QCoreApplication::applicationDirPath() + "/modules/";
}

PluginsManager* PluginsManager::getInstance()
{
	static PluginsManager pluginsManager;
	return &pluginsManager;
}

void PluginsManager::loadPlugins()
{
	QLibrary library("modules/PandaModules");
	library.load();
}

