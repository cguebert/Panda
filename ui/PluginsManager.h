#ifndef PLUGINSMANAGER_H
#define PLUGINSMANAGER_H

#include <QObject>
#include <QString>

class PluginsManager : public QObject
{
	Q_OBJECT
public:

	static PluginsManager* getInstance();

	void loadPlugins();

protected:
	PluginsManager();

	QString m_pluginsDirPath;
};

#endif // PLUGINSMANAGER_H
