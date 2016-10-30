#ifndef TIMEDFUNCTIONS_H
#define TIMEDFUNCTIONS_H

#include <panda/core.h>

#include <functional>
#include <memory>

namespace panda
{

// This class launchs a thread and accepts functions to run at a later time
class PANDA_CORE_API TimedFunctions
{
public:
	static void shutdown(); // Stop the thread

	using VoidFunc = std::function<void()>;
	static int delayRun(double seconds, VoidFunc func); // Returns an id that can be used to cancels the execution of the function. If delay is negative, run immediately and returns -1.

	static bool cancelRun(int index); // Return true if the execution has been cancelled, false if the function has already run or the index is invalid.
	static void cancelAll(); // Remove all functions, without shutting down the thread

private:
	TimedFunctions();
};

} // namespace panda

#endif // TIMEDFUNCTIONS_H
