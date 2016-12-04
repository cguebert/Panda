#ifndef PLUGINSMANAGER_H
#define PLUGINSMANAGER_H

#include <panda/core.h>

#include <memory>
#include <string>
#include <vector>

namespace panda
{
	class PluginsManager;

	class PANDA_CORE_API DynamicLibrary
	{
	public:
		using SPtr = std::shared_ptr<DynamicLibrary>;
		DynamicLibrary() = default;
		DynamicLibrary(const DynamicLibrary&) = default;
		DynamicLibrary(const std::string& path);

		bool load();
		bool unload(); // Not automatically called by the destructor

		bool isLoaded() const;
		const std::string& path() const;

	private:
		std::string m_path;
		void *m_handle = nullptr;
	};

	class PANDA_CORE_API PluginsManager
	{
	public:
		using PluginsList = std::vector<DynamicLibrary::SPtr>;

		static void loadPlugins();
		static const PluginsList& plugins();

	protected:
		static PluginsManager& instance();
		std::vector<DynamicLibrary::SPtr> m_plugins;
	};

}

#endif // PLUGINSMANAGER_H
