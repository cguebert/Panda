#include <panda/document/PandaDocument.h>
#include <panda/SimpleGUI.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>

#include <panda/helper/algorithm.h>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace panda {

class Filesystem_GetFilename: public PandaObject
{
public:
	PANDA_CLASS(Filesystem_GetFilename, PandaObject)

		Filesystem_GetFilename(PandaDocument *doc)
		: PandaObject(doc)
		, m_path(initData("path", "The path to analyse"))
		, m_directory(initData("directory", "Parent directory"))
		, m_filename(initData("filename", "File name"))
	{
		addInput(m_path);
		
		addOutput(m_directory);
		addOutput(m_filename);
	}

	void update()
	{
		const auto& paths = m_path.getValue();
		auto directory = m_directory.getAccessor();
		auto filename = m_filename.getAccessor();

		int nb = paths.size();
		directory.resize(nb);
		filename.resize(nb);

		for (int i = 0; i < nb; ++i)
		{
			fs::path file(paths[i]);

			if (fs::exists(file))
			{
				directory[i] = file.parent_path().string();
				filename[i] = file.filename().string();
			}
			else
				directory[i] = filename[i] = "";
		}
	}

protected:
	Data<std::vector<std::string>> m_path, m_directory, m_filename;
};

int Filesystem_GetFilenameClass = RegisterObject<Filesystem_GetFilename>("Modifier/Text/File/Get filename")
	.setDescription("Separate parent directory and file name");

//****************************************************************************//

class Filesystem_GetExtension : public PandaObject
{
public:
	PANDA_CLASS(Filesystem_GetExtension, PandaObject)

		Filesystem_GetExtension(PandaDocument *doc)
		: PandaObject(doc)
		, m_path(initData("path", "The path to analyse"))
		, m_stem(initData("stem", "The stem of the given file"))
		, m_extension(initData("extension", "The extension of the given file"))
	{
		addInput(m_path);

		addOutput(m_stem);
		addOutput(m_extension);
	}

	void update()
	{
		const auto& paths = m_path.getValue();
		auto stem = m_stem.getAccessor();
		auto extension = m_extension.getAccessor();

		int nb = paths.size();
		stem.resize(nb);
		extension.resize(nb);

		for (int i = 0; i < nb; ++i)
		{
			fs::path file(paths[i]);

			if (fs::exists(file))
			{
				stem[i] = file.stem().string();
				extension[i] = file.extension().string();
			}
			else
				stem[i] = extension[i] = "";
		}
	}

protected:
	Data<std::vector<std::string>> m_path, m_stem, m_extension;
};

int Filesystem_GetExtensionClass = RegisterObject<Filesystem_GetExtension>("Modifier/Text/File/Get extension")
	.setDescription("Separate stem and extension");

} // namespace Panda
