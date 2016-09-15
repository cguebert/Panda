#include <panda/document/PandaDocument.h>
#include <panda/object/ObjectFactory.h>

#include <FreeImage.h>
#include <sstream>

namespace panda {

class Images_Extensions : public PandaObject
{
public:
	PANDA_CLASS(Images_Extensions, PandaObject)

		Images_Extensions(PandaDocument *doc)
		: PandaObject(doc)
		, m_type(initData(0, "type", "Supported operation on the listed images"))
		, m_extensions(initData("extensions", "The list of extensions that support the given operation"))
	{
		addInput(m_type);
		m_type.setWidget("enum");
		m_type.setWidgetData("any;load;save;both");

		addOutput(m_extensions);
	}

	void update()
	{
		int type = m_type.getValue();
		auto extensions = m_extensions.getAccessor();
		extensions.clear();

		const int nb = FreeImage_GetFIFCount();
		for (int i = 0; i < nb; ++i)
		{
			auto fif = static_cast<FREE_IMAGE_FORMAT>(i);
			switch (type)
			{
			case 0: // Any
				break;
			case 1: // Load
				if (!FreeImage_FIFSupportsReading(fif))
					continue;
				break;
			case 2: // Save
				if (!FreeImage_FIFSupportsWriting(fif))
					continue;
				break;
			case 3: // Both
				if (!FreeImage_FIFSupportsReading(fif) || !FreeImage_FIFSupportsWriting(fif))
					continue;
				break;
			}

			std::stringstream ss(FreeImage_GetFIFExtensionList(fif));
			std::string item;
			while (std::getline(ss, item, ','))
				extensions.push_back(item);
		}
	}

protected:
	Data<int> m_type;
	Data<std::vector<std::string>> m_extensions;
};

int Images_ExtensionsClass = RegisterObject<Images_Extensions>("Generator/Text/File/Images extensions")
	.setDescription("Obtain the list of supported image files extensions");

} // namespace Panda
