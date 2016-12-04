#pragma once

#include <panda/core.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace panda
{
	class PandaDocument;
	class PandaObject;

	namespace gui {
		class BaseGUI;
	}

	class PANDA_CORE_API VisualizersManager
	{
	public:
		struct VisualizerInformation
		{
			std::string path, name;
			int dataType = 0;
		};
		using Visualizers = std::vector<VisualizerInformation>;

		static void createList();
		static const Visualizers& visualizers();
		static Visualizers visualizers(int dataType);
		static std::unique_ptr<panda::PandaDocument> createVisualizer(const std::string& name, panda::gui::BaseGUI& gui);

	protected:
		VisualizersManager();
		static VisualizersManager& instance();

		Visualizers m_list;
		static const std::string m_dirPath;
	};

}
