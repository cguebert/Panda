#include <panda/object/ObjectFactory.h>
#include <panda/SimpleGUI.h>

#include <modules/Images/utils.h>

#include <algorithm>
#include <atomic>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>

namespace panda {

using types::ImageWrapper;

using ComputationFunc = std::function<graphics::Image(const std::string& path)>;

class LoadingThread
{
public:
	LoadingThread(PandaObject* parent)
		: m_parentObject(parent)
	{
		m_isWorking = false;
	}

	~LoadingThread()
	{
		{
			std::unique_lock<std::mutex> lock(m_inputMutex);
			m_pathsToLoad.clear();
		}

		if(m_thread)
			m_thread->join();
	}

	bool isWorking()
	{
		return m_isWorking;
	}

	void loadImages(const std::vector<std::string>& paths) // Replace what must be loaded
	{
		{
			std::unique_lock<std::mutex> lock(m_inputMutex);
			m_pathsToLoad.clear();
			m_pathsToLoad.insert(m_pathsToLoad.end(), paths.begin(), paths.end());
		}

		// The previous thread has finished working, but we still have its handle
		if (m_thread && !m_isWorking)
		{
			m_thread->detach();
			m_thread.reset();
		}

		// Launch a new thread
		if (!m_thread && m_computationFunc)
			m_thread = std::make_unique<std::thread>(&LoadingThread::loadingFunc, this);
	}

	void setComputationFunc(ComputationFunc func)
	{
		if (!m_isWorking)
			m_computationFunc = func;
	}

	bool getImage(const std::string& path, graphics::Image& img) // Returns true if the image is ready and img has been modified
	{
		std::unique_lock<std::mutex> lock(m_outputMutex);
		auto it = std::find_if(m_readyImages.begin(), m_readyImages.end(), [&path](const ReadyPair& rp) {
			return rp.first == path;
		});
		if (it != m_readyImages.end())
		{
			img = it->second;
			m_readyImages.erase(it);
			return true;
		}
		return false;
	}

	void clean() // Ensure all images are freed
	{
		{
			std::unique_lock<std::mutex> lock(m_inputMutex);
			m_pathsToLoad.clear();
		}

		{
			std::unique_lock<std::mutex> lock(m_outputMutex);
			m_readyImages.clear();
		}
	}

	void join() // Wait for the loading to be finished
	{
		if (m_thread)
		{
			m_thread->join();
			m_thread.reset();
		}
	}

	void loadingFunc()
	{
		m_isWorking = true;
		while (true)
		{
			std::string path;
			{
				std::unique_lock<std::mutex> lock(m_inputMutex);
				if (m_pathsToLoad.empty())
				{
					m_isWorking = false;
					return;
				}
				path = m_pathsToLoad.front();
				m_pathsToLoad.pop_front();
			}

			// Do the actual loading
			auto img = m_computationFunc(path);

			// Set the parent object as dirty, so that it updates again
			auto object = m_parentObject;
			m_parentObject->parentDocument()->getGUI().executeByUI([object]() {
				object->setDirtyValue(nullptr);
			});

			{
				std::unique_lock<std::mutex> lock(m_outputMutex);
				m_readyImages.emplace_back(std::move(path), std::move(img));
			}
		}
	}

private:
	PandaObject* m_parentObject = nullptr;
	std::deque<std::string> m_pathsToLoad;
	using ReadyPair = std::pair<std::string, graphics::Image>;
	std::deque<ReadyPair> m_readyImages;
	std::mutex m_inputMutex, m_outputMutex;
	std::unique_ptr<std::thread> m_thread;
	std::atomic_bool m_isWorking;
	ComputationFunc m_computationFunc;
};

//****************************************************************************//

class GeneratorImage_Load : public PandaObject
{
public:
	PANDA_CLASS(GeneratorImage_Load, PandaObject)

	GeneratorImage_Load(PandaDocument *doc)
		: PandaObject(doc)
		, m_fileName(initData("fileName", "Path of the image to load"))
		, m_asynchronous(initData(0, "asynchronous", "If true, load the image in the background. The output is null until the loading is finished."))
		, m_image(initData("image", "The image loaded from disk"))
		, m_loadingThread(this)
	{
		addInput(m_fileName);
		
		m_fileName.setWidget("open file");
		m_fileName.setWidgetData(getOpenFilterString());

		// The "asynchronous" data is not set as an input, but appears in the GUI
		m_asynchronous.setWidget("checkbox");

		addOutput(m_image);

		m_loadingThread.setComputationFunc([](const std::string& path) { return loadImage(path); });
	}

	void update()
	{
		const auto& path = m_fileName.getValue();
		auto image = m_image.getAccessor();
		
		if (m_asynchronous.getValue() != 0 || m_loadingThread.isWorking())
		{
			int counter = m_fileName.getCounter();
			if (counter != m_fileNameLastCounter) // Begin the loading
			{
				image->clear();
				m_fileNameLastCounter = counter;
				m_loadingThread.clean();
				m_loadingThread.loadImages({ path });
				return;
			}
			else // Look for loaded images
			{
				graphics::Image img;
				if (image->isNull() && m_loadingThread.getImage(path, img))
					image->setImage(img);
			}
		}
		else
		{
			auto img = loadImage(m_fileName.getValue());
			if(img)
				m_image.getAccessor()->setImage(img);
			else
				m_image.getAccessor()->clear();
		}
	}

protected:
	Data<std::string> m_fileName;
	Data<int> m_asynchronous;
	Data<ImageWrapper> m_image;

	LoadingThread m_loadingThread;
	int m_fileNameLastCounter = -1;
};

int GeneratorImage_LoadClass = RegisterObject<GeneratorImage_Load>("File/Image/Load image").setDescription("Load an image from the disk");

//****************************************************************************//

class BaseGeneratorImage_LoadMultiple : public PandaObject
{
public:
	PANDA_CLASS(BaseGeneratorImage_LoadMultiple, PandaObject)

	BaseGeneratorImage_LoadMultiple(PandaDocument *doc)
		: PandaObject(doc)
		, m_fileName(initData("fileName", "Path of the image to load"))
		, m_asynchronous(initData(0, "asynchronous", "If true, load the image in the background. The output is null until the loading is finished."))
		, m_image(initData("image", "The image loaded from disk"))
		, m_loadingThread(this)
	{
		addInput(m_fileName);
		m_fileName.setWidget("open file");
		m_fileName.setWidgetData(getOpenFilterString());

		// The "asynchronous" data is not set as an input, but appears in the GUI
		m_asynchronous.setWidget("checkbox");

		addOutput(m_image);
	}

	bool inputsHaveChanged()
	{
		auto inputs = getInputDatas();
		if (inputs.size() != m_counters.size()) // First time (or added/removed an input)
		{
			m_counters.clear();
			for (const auto& input : inputs)
				m_counters.emplace_back(input, input->getCounter());
			return true;
		}

		bool changed = false;
		for (auto& c : m_counters)
		{
			int counter = c.first->getCounter();
			if (c.second != counter)
				changed = true;
			c.second = counter;
		}

		return changed;
	}

	void update()
	{
		const auto& paths = m_fileName.getValue();
		auto images = m_image.getAccessor();
		int nb = paths.size();
		
		if (m_asynchronous.getValue() != 0 || m_loadingThread.isWorking())
		{
			if (inputsHaveChanged()) // Begin the loading
			{
				images.clear();
				images.resize(nb);
				m_loadingThread.clean();
				m_loadingThread.setComputationFunc(std::move(getComputationFunc()));
				m_loadingThread.loadImages(paths);
				return;
			}
			else // Look for loaded images
			{
				graphics::Image img;
				for (int i = 0; i < nb; ++i)
				{
					if (images[i].isNull())
					{
						if (m_loadingThread.getImage(paths[i], img))
							images[i].setImage(img);
						else
							break;
					}
				}
			}
		}
		else
		{
			auto computeFunc = getComputationFunc();
			images.resize(nb);

			for (int i = 0; i < nb; ++i)
			{
				auto img = computeFunc(paths[i]);
				if (img)
					images[i].setImage(img);
				else
					images[i].clear();
			}
		}
	}

protected:
	virtual ComputationFunc getComputationFunc() = 0;

	Data<std::vector<std::string>> m_fileName;
	Data<int> m_asynchronous;
	Data<std::vector<ImageWrapper>> m_image;

	LoadingThread m_loadingThread;
	std::vector<std::pair<BaseData*, int>> m_counters;

	ComputationFunc m_computationFunc;
};

//****************************************************************************//

class GeneratorImage_LoadMultiple : public BaseGeneratorImage_LoadMultiple
{
public:
	PANDA_CLASS(GeneratorImage_LoadMultiple, BaseGeneratorImage_LoadMultiple)

	GeneratorImage_LoadMultiple(PandaDocument *doc)
		: BaseGeneratorImage_LoadMultiple(doc)
	{ }

	ComputationFunc getComputationFunc() override
	{ 
		return [](const std::string& path) { 
			return loadImage(path); 
		};
	}
};

int GeneratorImage_LoadMultipleClass = RegisterObject<GeneratorImage_LoadMultiple>("File/Image/Load images").setDescription("Load multiple images from the disk");

//****************************************************************************//

class GeneratorImage_LoadThumbnails : public BaseGeneratorImage_LoadMultiple
{
public:
	PANDA_CLASS(GeneratorImage_LoadThumbnails, BaseGeneratorImage_LoadMultiple)

	GeneratorImage_LoadThumbnails(PandaDocument *doc)
		: BaseGeneratorImage_LoadMultiple(doc)
	{
		m_fileName.setWidgetData(getThumbnailFilterString());
	}

	ComputationFunc getComputationFunc() override
	{ 
		return &GeneratorImage_LoadThumbnails::loadThumbnail;
	}

	static std::string getThumbnailFilterString()
	{
		static std::string filter;
		if (filter.empty())
		{
			filter = "All Files (*);;";
			filter += getFilterString({ FIF_JPEG, FIF_PSD, FIF_EXR, FIF_TARGA, FIF_TIFF });
		}

		return filter;
	}

	static graphics::Image loadThumbnail(const std::string& path)
	{
		if (!path.empty())
			return graphics::Image();
			
		graphics::Image img;
		auto cpath = path.c_str();
		auto fif = FreeImage_GetFileType(cpath, 0);
		if (fif == FIF_UNKNOWN)
			fif = FreeImage_GetFIFFromFilename(cpath);
		if (fif != FIF_UNKNOWN && FreeImage_FIFSupportsReading(fif))
		{
			auto dib = FreeImage_Load(fif, cpath, FIF_LOAD_NOPIXELS);

			if (dib)
			{
				auto thumbnail = FreeImage_GetThumbnail(dib);
				if(thumbnail)
					img = convertTo32bitsImage(thumbnail);
				FreeImage_Unload(dib); // thumbnail is a child of dib, and freed by it
			}
		}
		
		return img;
	}
};

int GeneratorImage_LoadThumbnailClass = RegisterObject<GeneratorImage_LoadThumbnails>("File/Image/Load thumbnails")
	.setDescription("Load the thumbnail from multiple images (null output if it does not exist)");

//****************************************************************************//

class GeneratorImage_LoadOrMakeThumbnails : public GeneratorImage_LoadThumbnails
{
public:
	PANDA_CLASS(GeneratorImage_LoadOrMakeThumbnails, GeneratorImage_LoadThumbnails)

	GeneratorImage_LoadOrMakeThumbnails(PandaDocument *doc)
		: GeneratorImage_LoadThumbnails(doc)
		, m_size(initData(100, "size", "Maximum size of the created thumbnails"))
		, m_loadExisting(initData(true, "loadExisting", "If a thumbnail is bundled in the file, use it"))
	{
		addInput(m_size);
		addInput(m_loadExisting);

		m_loadExisting.setWidget("checkbox");

		addData(&m_asynchronous); // Put it in the back again
	}

	ComputationFunc getComputationFunc() override
	{ 
		const int size = m_size.getValue();
		bool loadExisting = (m_loadExisting.getValue() != 0);
		return [size, loadExisting](const std::string& path) { 
			return loadOrMakeThumbnail(path, size, loadExisting); 
		};
	}

	static graphics::Image loadOrMakeThumbnail(const std::string& path, int size, bool loadExisting)
	{
		auto cpath = path.c_str();
		auto fif = FreeImage_GetFileType(cpath, 0);
		if (fif == FIF_UNKNOWN)
			fif = FreeImage_GetFIFFromFilename(cpath);
		if (fif == FIF_UNKNOWN || !FreeImage_FIFSupportsReading(fif))
			return graphics::Image();

		// First try loading the thumbnail
		if (loadExisting)
		{
			if (FreeImage_FIFSupportsNoPixels(fif))
			{
				auto dib = FreeImage_Load(fif, cpath, FIF_LOAD_NOPIXELS);
				if (dib)
				{
					auto thumbnail = FreeImage_GetThumbnail(dib);
					if (thumbnail)
					{
						auto img = convertTo32bitsImage(thumbnail);
						FreeImage_Unload(dib); // thumbnail is a child of dib, and freed by it
						return img;
					}
				}
			}
		}

		FIBITMAP* dib = nullptr;
		// Make the thumbnail
		if (fif == FIF_JPEG)
		{
			FITAG* tag = nullptr;
			// Downsampling while loading
			dib = FreeImage_Load(fif, cpath, size << 16);
		}
		else
		{
			int flag = 0;
			if (fif == FIF_RAW)
				flag = RAW_PREVIEW;
			dib = FreeImage_Load(fif, cpath, flag);
		}

		if (dib)
		{
			auto thumbnail = FreeImage_MakeThumbnail(dib, size, TRUE);
			auto img = convertTo32bitsImage(thumbnail);
			FreeImage_Unload(dib); // thumbnail is a child of dib, and freed by it
			return img;
		}

		return graphics::Image();
	}

protected:
	Data<int> m_size, m_loadExisting;
};

int GeneratorImage_LoadOrMakeThumbnailsClass = RegisterObject<GeneratorImage_LoadOrMakeThumbnails>("File/Image/Load or make thumbnails")
	.setDescription("Load of make the thumbnail for multiple images");

} // namespace Panda
