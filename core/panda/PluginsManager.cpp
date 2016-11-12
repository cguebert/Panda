#include <panda/PluginsManager.h>
#include <panda/object/ObjectFactory.h>
#include <panda/helper/system/FileRepository.h>

#include <iostream>

#ifdef WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

namespace panda
{

	DynamicLibrary::DynamicLibrary(const std::string& path)
		: m_path(path)
	{
	}
	
	bool DynamicLibrary::load()
	{
		if (isLoaded())
			return false;

	# if defined(WIN32)
		m_handle = ::LoadLibraryA(m_path.c_str());
	# else
		m_handle = ::dlopen(m_path.c_str(), RTLD_NOW);
	# endif
		
		return isLoaded();
	}

	bool DynamicLibrary::unload()
	{
		auto handle = m_handle;
		m_handle = nullptr;
	# if defined(WIN32)
		return 0 != FreeLibrary((HMODULE)handle);
	# else
		return 0 == ::dlclose(handle);
	# endif
	}

	bool DynamicLibrary::isLoaded() const
	{
		return m_handle != nullptr;
	}

	const std::string& DynamicLibrary::path() const
	{
		return m_path;
	}

//****************************************************************************//

	PluginsManager* PluginsManager::getInstance()
	{
		static PluginsManager pluginsManager;
		return &pluginsManager;
	}

	void PluginsManager::loadPlugins(const std::string& directory)
	{
		auto& factory = panda::ObjectFactory::instance();
		factory.moduleLoaded(); // Register core modules

#if defined(WIN32)
		const std::string filter = ".dll";
#elif defined(__APPLE__)
		const std::string filter = ".dylib";
#else
		const std::string filter = ".so";
#endif

		auto modules = panda::helper::system::DataRepository.enumerateFilesInDir(directory, filter);
		for (const auto& moduleName : modules)
		{
			auto absolutePath = panda::helper::system::DataRepository.findFile(directory + "/" + moduleName);
			auto library = std::make_shared<DynamicLibrary>(absolutePath);

			if (library->load())
			{
				m_plugins.push_back(library);
				factory.moduleLoaded();
			}
			else
				std::cerr << "Could not load library " << moduleName << std::endl;
		}

		factory.allObjectsRegistered();
	}

}
