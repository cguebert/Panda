#include <panda/helper/system/FileRepository.h>
#include <panda/helper/algorithm.h>

#include <fstream>

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

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

std::string FileRepository::findFile(const std::string& fileName) const
{
	fs::path filePath(fileName);
	if(filePath.is_absolute())
	{
		if(exists(filePath))
			return fileName;
		else
			return std::string();
	}

	for(const auto& path : m_paths)
	{
		fs::path dir(path);
		filePath = dir / fileName;
		if(exists(filePath))
			return absolute(filePath).lexically_normal().string();
	}

	return std::string();
}

std::string FileRepository::loadFile(const std::string& fileName) const
{
	std::string contents;
	std::ifstream in(findFile(fileName), std::ios_base::binary);
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

std::vector<std::string> FileRepository::enumerateFilesInDir(const std::string& dirPath, const std::string& extension)
{
	std::vector<std::string> files;
	fs::path dir(dirPath);
	if(dir.is_absolute() && exists(dir) && is_directory(dir))
	{
		for (auto&& x : fs::recursive_directory_iterator(dir))
		{
			if (is_regular_file(x) && (extension.empty() || x.path().extension() == extension))
				files.push_back(x.path().lexically_relative(dirPath).generic_string());
		}
		return files;
	}

	for(const std::string& path : m_paths)
	{
		dir = fs::path(path) / dirPath;
		if (exists(dir) && is_directory(dir))
		{
			for (auto&& x : fs::recursive_directory_iterator(dir))
			{
				if (is_regular_file(x) && (extension.empty() || x.path().extension() == extension))
					files.push_back(x.path().lexically_relative(dir.string()).generic_string());
			}
		}
	}
	std::sort(files.begin(), files.end());
	auto last = std::unique(files.begin(), files.end());
	files.erase(last, files.end());
	return files;
}

FileRepository DataRepository;

} // namespace system

} // namespace helper

} // namespace panda
