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

	/// Returns the path to the file if it can be found in the repository
	std::string findFile(const std::string& fileName) const;

	/// Looks for the file in the repository, loads it and returns its contents (empty string if the file cannot be found)
	std::string loadFile(const std::string& fileName) const;

	/// List the relative paths of files in all the listed directories
	std::vector<std::string> enumerateFilesInDir(const std::string& dir, const std::string& extension = std::string());

protected:
	std::vector<std::string> m_paths;
};

extern PANDA_CORE_API FileRepository DataRepository; /// Default repository

} // namespace system

} // namespace helper

} // namespace panda

#endif // HELPER_SYSTEM_FILEREPOSITORY_H
