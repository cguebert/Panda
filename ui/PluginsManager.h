#ifndef PLUGINSMANAGER_H
#define PLUGINSMANAGER_H

#include <QLibrary>

#include <memory>
#include <vector>

class PluginsManager
{
public:
	static PluginsManager* getInstance();

	void loadPlugins();

protected:
	typedef std::shared_ptr<QLibrary> LibraryPtr;
	std::vector<LibraryPtr> m_plugins;
};

#endif // PLUGINSMANAGER_H
