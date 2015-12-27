#ifndef HELPER_SYSTEM_FILEREPOSITORY_H
#define HELPER_SYSTEM_FILEREPOSITORY_H

#include <panda/core.h>

#include <QByteArray>
#include <QString>
#include <QStringList>

#include <vector>

namespace panda
{

namespace helper
{

namespace system
{

// This class is used to search for files in multiple directories:
// User home, app data dir, app dir
class PANDA_CORE_API FileRepository
{
public:
	void addPath(QString path);

	QString findFile(const QString& fileName);
	QByteArray loadFile(const QString& fileName);

	/// List the relative paths of files in all the listed directories
	QStringList enumerateFilesInDir(const QString& dir, const QString& nameFilter = QString());

protected:
	std::vector<QString> m_paths;
};

extern PANDA_CORE_API FileRepository DataRepository; /// Default repository

} // namespace system

} // namespace helper

} // namespace panda

#endif // HELPER_SYSTEM_FILEREPOSITORY_H
