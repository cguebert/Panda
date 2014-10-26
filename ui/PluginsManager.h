#ifndef PLUGINSMANAGER_H
#define PLUGINSMANAGER_H

#include <QLibrary>
#include <QSharedPointer>
#include <QVector>

class PluginsManager
{
public:
	static PluginsManager* getInstance();

	void loadPlugins();

protected:
	typedef QSharedPointer<QLibrary> LibraryPtr;
	QVector<LibraryPtr> m_plugins;
};

#endif // PLUGINSMANAGER_H
