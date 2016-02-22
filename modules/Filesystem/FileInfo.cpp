#include <panda/PandaDocument.h>
#include <panda/SimpleGUI.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>

#include <panda/helper/algorithm.h>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace panda {

class Filesystem_FileType : public PandaObject
{
public:
	PANDA_CLASS(Filesystem_FileType, PandaObject)

		Filesystem_FileType(PandaDocument *doc)
		: PandaObject(doc)
		, m_path(initData("path", "The path to analyse"))
		, m_exists(initData("exists", "Does the given path point to something"))
		, m_isDir(initData("isDir", "Does the given path point to a directory"))
		, m_isFile(initData("isFile", "Does the given path point to a regular file"))
	{
		addInput(m_path);

		addOutput(m_exists);
		addOutput(m_isDir);
		addOutput(m_isFile);
	}

	void update()
	{
		const auto& paths = m_path.getValue();
		auto exists = m_exists.getAccessor();
		auto isDir = m_isDir.getAccessor();
		auto isFile = m_isFile.getAccessor();

		int nb = paths.size();
		exists.resize(nb);
		isDir.resize(nb);
		isFile.resize(nb);

		for (int i = 0; i < nb; ++i)
		{
			fs::path file(paths[i]);

			if (fs::exists(file))
			{
				exists[i] = true;
				isDir[i] = fs::is_directory(file);
				isFile[i] = fs::is_regular_file(file);
			}
			else
				exists[i] = isDir[i] = isFile[i] = false;
		}

		cleanDirty();
	}

protected:
	Data<std::vector<std::string>> m_path;
	Data<std::vector<int>> m_exists, m_isDir, m_isFile;
};

int Filesystem_FileTypeClass = RegisterObject<Filesystem_FileType>("Modifier/Text/File/File type")
	.setDescription("Get the file type");

//****************************************************************************//

class Filesystem_FileSize : public PandaObject
{
public:
	PANDA_CLASS(Filesystem_FileSize, PandaObject)

		Filesystem_FileSize(PandaDocument *doc)
		: PandaObject(doc)
		, m_path(initData("path", "The path to analyse"))
		, m_size(initData("size", "The size of the file (-1 if not found)"))
	{
		addInput(m_path);

		addOutput(m_size);
	}

	void update()
	{
		const auto& paths = m_path.getValue();
		auto size = m_size.getAccessor();

		int nb = paths.size();
		size.resize(nb);

		for (int i = 0; i < nb; ++i)
		{
			fs::path file(paths[i]);
			size[i] = static_cast<int>(fs::file_size(file));
		}

		cleanDirty();
	}

protected:
	Data<std::vector<std::string>> m_path;
	Data<std::vector<int>> m_size;
};

int Filesystem_FileSizeClass = RegisterObject<Filesystem_FileSize>("Modifier/Text/File/File size")
	.setDescription("Get the file size");

} // namespace Panda
