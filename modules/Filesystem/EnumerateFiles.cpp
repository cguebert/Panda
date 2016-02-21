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

		m_recursive.setWidget("checkbox");

		addOutput(m_files);
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
				for (auto&& x : fs::recursive_directory_iterator(dir))
				{
					if (extensions.empty() || helper::contains(extensions, x.path().extension().string()))
						files.push_back(x.path().string());
				}
			}
			else
			{
				for (auto&& x : fs::directory_iterator(dir))
				{
					if (extensions.empty() || helper::contains(extensions, x.path().extension().string()))
						files.push_back(x.path().string());
				}
			}
		}

		cleanDirty();
	}

protected:
	Data<std::string> m_directory;
	Data<std::vector<std::string>> m_extensions, m_files;
	Data<int> m_recursive;
};

int Filesystem_EnumerateFilesClass = RegisterObject<Filesystem_EnumerateFiles>("Generator/Text/File/Enumerate files")
.setDescription("Enumerate files in a directory");

//****************************************************************************//

} // namespace Panda
