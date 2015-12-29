#include <panda/helper/system/FileRepository.h>
#include <panda/helper/algorithm.h>

#include <fstream>

#include <QStandardPaths>
#include <QFile>
#include <QDir>

namespace
{

std::vector<std::string> convert(const QStringList& list)
{
	std::vector<std::string> result;
	for (const auto& s : list)
		result.push_back(s.toStdString());
	return result;
}

}

namespace panda
{

namespace helper
{

namespace system
{

void FileRepository::addPath(const std::string& path)
{
	if(!helper::contains(m_paths, path))
		m_paths.push_back(path);
}

std::string FileRepository::findFile(const std::string& fileName)
{
	auto qFileName = QString::fromStdString(fileName);
	if(QDir::isAbsolutePath(qFileName))
	{
		if(QFile::exists(qFileName))
			return fileName;
		else
			return std::string();
	}

	for(const std::string& path : m_paths)
	{
		QDir dir(QString::fromStdString(path));
		if(dir.exists(dir.filePath(qFileName)))
			return QDir::cleanPath(dir.absoluteFilePath(qFileName)).toStdString();
	}

	return std::string();
}

std::string FileRepository::loadFile(const std::string& fileName)
{
	std::string contents;
	std::ifstream in(fileName, std::ios_base::binary);
	if (in)
	{
		in.seekg(0, std::ios::end);
		contents.resize((size_t)in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&contents[0], contents.size());
		in.close();
	}
	return contents;
}

std::vector<std::string> FileRepository::enumerateFilesInDir(const std::string& dirPath, const std::string& nameFilter)
{
	auto qNameFilter = QString::fromStdString(nameFilter);
	auto qDirPath = QString::fromStdString(dirPath);
	QStringList nameFilters(qNameFilter);
	if(QDir::isAbsolutePath(qDirPath))
	{
		QDir dir(qDirPath);
		if(qNameFilter.isEmpty())
			return convert(dir.entryList(QDir::Files, QDir::Name));
		else
			return convert(dir.entryList(nameFilters, QDir::Files, QDir::Name));
	}

	QStringList result;
	for(const std::string& path : m_paths)
	{
		QDir dir(QString::fromStdString(path));
		dir.setPath(qDirPath);
		if(qNameFilter.isEmpty())
			result.append(dir.entryList(QDir::Files, QDir::Name));
		else
			result.append(dir.entryList(nameFilters, QDir::Files, QDir::Name));
	}
	result.sort(Qt::CaseInsensitive);
	result.removeDuplicates();
	return convert(result);
}

FileRepository DataRepository;

} // namespace system

} // namespace helper

} // namespace panda
