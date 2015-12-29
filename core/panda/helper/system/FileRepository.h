#ifndef HELPER_SYSTEM_FILEREPOSITORY_H
#define HELPER_SYSTEM_FILEREPOSITORY_H

#include <panda/core.h>

#include <string>
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
	void addPath(const std::string& path);

	std::string findFile(const std::string& fileName);
	std::string loadFile(const std::string& fileName);

	/// List the relative paths of files in all the listed directories
	std::vector<std::string> enumerateFilesInDir(const std::string& dir, const std::string& nameFilter = std::string());

protected:
	std::vector<std::string> m_paths;
};

extern PANDA_CORE_API FileRepository DataRepository; /// Default repository

} // namespace system

} // namespace helper

} // namespace panda

#endif // HELPER_SYSTEM_FILEREPOSITORY_H
