#include <panda/PandaDocument.h>
#include <panda/SimpleGUI.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>

#include <panda/helper/algorithm.h>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace panda {

class Filesystem_EnumerateFiles : public PandaObject
{
public:
	PANDA_CLASS(Filesystem_EnumerateFiles, PandaObject)

		Filesystem_EnumerateFiles(PandaDocument *doc)
		: PandaObject(doc)
		, m_directory(initData("directory", "The directory to analyse"))
		, m_extensions(initData("extensions", "If not empty, only show files having these extensions"))
		, m_files(initData("files", "The files found in the directory"))
		, m_recursive(initData(false, "recursive", "If true, the search will be recursive in child directories"))
	{
		addInput(m_directory);
		addInput(m_extensions);
		addInput(m_recursive);

		m_directory.setWidget("directory");
		m_recursive.setWidget("checkbox");

		addOutput(m_files);
	}

	void addFile(std::vector<std::string>& files, fs::path path, const std::vector<std::string>& extensions)
	{
		if (!fs::is_regular_file(path))
			return;
		
		auto ext = path.extension().string();
		if (!ext.empty())
			ext = ext.substr(1); // Remove the '.'
		if(extensions.empty() || helper::contains(extensions, ext))
			files.push_back(path.string());
	}

	void update()
	{
		fs::path dir(m_directory.getValue());
		auto& extensions = m_extensions.getValue();
		auto files = m_files.getAccessor();
		files.clear();
		if (dir.is_absolute() && exists(dir) && is_directory(dir))
		{
			if (m_recursive.getValue())
			{
				for (auto& x : fs::recursive_directory_iterator(dir))
					addFile(files.wref(), x.path(), extensions);
			}
			else
			{
				for (auto& x : fs::directory_iterator(dir))
					addFile(files.wref(), x.path(), extensions);
			}
		}
	}

protected:
	Data<std::string> m_directory;
	Data<std::vector<std::string>> m_extensions, m_files;
	Data<int> m_recursive;
};

int Filesystem_EnumerateFilesClass = RegisterObject<Filesystem_EnumerateFiles>("Generator/Text/File/Enumerate files")
.setDescription("Enumerate files in a directory");

//****************************************************************************//

class Filesystem_EnumerateDirectories : public PandaObject
{
public:
	PANDA_CLASS(Filesystem_EnumerateDirectories, PandaObject)

		Filesystem_EnumerateDirectories(PandaDocument *doc)
		: PandaObject(doc)
		, m_directory(initData("directory", "The directory to analyse"))
		, m_directories(initData("files", "The files found in the directory"))
	{
		addInput(m_directory);
		addOutput(m_directories);

		m_directory.setWidget("directory");
	}

	void update()
	{
		fs::path dir(m_directory.getValue());
		auto dirs = m_directories.getAccessor();
		dirs.clear();
		if (dir.is_absolute() && exists(dir) && is_directory(dir))
		{
			for (auto& x : fs::directory_iterator(dir))
			{
				if (fs::is_directory(x))
					dirs.push_back(x.path().string());
			}
		}
	}

protected:
	Data<std::string> m_directory;
	Data<std::vector<std::string>> m_directories;
};

int Filesystem_EnumerateDirectoriesClass = RegisterObject<Filesystem_EnumerateDirectories>("Generator/Text/File/Enumerate directories")
.setDescription("Enumerate child directories");

} // namespace Panda
