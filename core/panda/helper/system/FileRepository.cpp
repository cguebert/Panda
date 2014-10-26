#include <panda/helper/system/FileRepository.h>

#include <QStandardPaths>
#include <QFile>
#include <QDir>

namespace panda
{

namespace helper
{

namespace system
{

void FileRepository::addPath(QString path)
{
	m_paths.push_back(path);
}

QString FileRepository::findFile(const QString& fileName)
{
	if(QDir::isAbsolutePath(fileName))
	{
		if(QFile::exists(fileName))
			return fileName;
		else
			return QString();
	}

	for(const QString& path : m_paths)
	{
		QDir dir(path);
		if(dir.exists(dir.filePath(fileName)))
			return QDir::cleanPath(dir.absoluteFilePath(fileName));
	}

	return QString();
}

QByteArray FileRepository::loadFile(const QString& fileName)
{
	QString path = findFile(fileName);
	if(!path.isEmpty())
	{
		QFile file(path);
		if(file.open(QIODevice::ReadOnly))
			return file.readAll();
	}

	return QByteArray();
}

QStringList FileRepository::enumerateFilesInDir(const QString& dirPath, const QString& nameFilter)
{
	QStringList nameFilters(nameFilter);
	if(QDir::isAbsolutePath(dirPath))
	{
		QDir dir(dirPath);
		if(nameFilter.isEmpty())
			return dir.entryList(QDir::Files, QDir::Name);
		else
			return dir.entryList(nameFilters, QDir::Files, QDir::Name);
	}

	QStringList result;
	for(const QString& path : m_paths)
	{
		QDir dir(path);
		dir.setPath(dirPath);
		if(nameFilter.isEmpty())
			result.append(dir.entryList(QDir::Files, QDir::Name));
		else
			result.append(dir.entryList(nameFilters, QDir::Files, QDir::Name));
	}
	result.sort(Qt::CaseInsensitive);
	result.removeDuplicates();
	return result;
}

FileRepository DataRepository;

} // namespace system

} // namespace helper

} // namespace panda
