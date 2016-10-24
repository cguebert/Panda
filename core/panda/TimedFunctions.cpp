#include <panda/TimedFunctions.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <deque>

using namespace std::chrono;

namespace panda
{

class TimedFunctionsData
{
public:
	TimedFunctionsData();

	void shutdown();
	int add(double delay, TimedFunctions::VoidFunc func);
	bool remove(int index);
	void removeAll();

private:
	void threadFunc();

	struct FuncData
	{
		int index;
		high_resolution_clock::time_point time;
		TimedFunctions::VoidFunc function;
	};

	int m_index = 0;
	std::deque<FuncData> m_functions;
	std::mutex m_mutex;
	std::atomic_bool m_running;
	std::condition_variable m_condition;
};

TimedFunctionsData::TimedFunctionsData()
{
	m_running = true;
	std::thread(&TimedFunctionsData::threadFunc, this).detach();
}

void TimedFunctionsData::shutdown()
{
	m_running = false;
	m_condition.notify_one();
}

int TimedFunctionsData::add(double delay, TimedFunctions::VoidFunc func)
{
	high_resolution_clock::time_point time = high_resolution_clock::now() + microseconds(std::lround(delay * 1e6));

	std::lock_guard<std::mutex> lock(m_mutex);

	// Find the first function that will be executed after the one we want to insert
	auto it = std::find_if(m_functions.begin(), m_functions.end(), [&time](const FuncData& func) {
		return func.time > time;
	});

	FuncData data;
	int id = ++m_index;
	if (id < 0)
		m_index = id = 1;
	data.index = id;
	data.time = time;
	data.function = std::move(func);
	m_functions.emplace(it, data);

	m_condition.notify_one();

	return id;
}

bool TimedFunctionsData::remove(int index)
{
	std::lock_guard<std::mutex> lock(m_mutex);

	auto it = std::find_if(m_functions.begin(), m_functions.end(), [&index](const FuncData& func) {
		return func.index == index;
	});

	bool ret = false;
	if (it != m_functions.end())
	{
		m_functions.erase(it);
		ret = true;
		m_condition.notify_one();
	}

	return ret;
}

void TimedFunctionsData::removeAll()
{
	std::lock_guard<std::mutex> lock(m_mutex);

	if (!m_functions.empty())
	{
		m_functions.clear();
		m_condition.notify_one();
	}
}

void TimedFunctionsData::threadFunc()
{
	while (m_running)
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		if (m_functions.empty()) // Either wait until there is a function
		{
			m_condition.wait(lock, [this]() {
				return !m_running || !m_functions.empty();
			});
		}
		else // Or wait until the function can be executed
		{
			high_resolution_clock::time_point nextTime = m_functions.front().time;
			m_condition.wait_until(lock, nextTime, [this, nextTime]() {
				if (!m_running || m_functions.empty()) // If remove has been called while waiting
					return true;
				
				auto time = m_functions.front().time;
				if (time < nextTime) // Another function has been inserted, we need to change the sleep
					return true;

				// Test if we can execute the next function
				return high_resolution_clock::now() >= time;
			});

			if (!m_functions.empty() && m_running)
			{
				if (high_resolution_clock::now() < m_functions.front().time) // If we inserted another function, but we still need to wait
					continue;

				// Pop the function
				const FuncData funcData = m_functions.front();
				m_functions.pop_front();

				lock.unlock(); // Unlock the mutex so other threads can add or remove functions while we execute the function
				funcData.function(); // Execute the function
			}
		}
	}
}

//****************************************************************************//

TimedFunctions::TimedFunctions()
	: m_data(std::make_shared<TimedFunctionsData>())
{ }

TimedFunctions& TimedFunctions::instance()
{ 
	static TimedFunctions tf; 
	return tf; 
}

void TimedFunctions::shutdown()
{ 
	m_data->shutdown(); 
}

int TimedFunctions::delayRun(double delay, VoidFunc func)
{
	if (delay <= 0)
	{
		func();
		return -1;
	}
	else
		return m_data->add(delay, std::move(func)); 
}

bool TimedFunctions::cancelRun(int index)
{
	if (index < 0)
		return false;
	return m_data->remove(index); 
}

void TimedFunctions::cancelAll()
{
	m_data->removeAll();
}

} // namespace panda
