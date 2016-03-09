#include <panda/types/Path.h>

namespace panda 
{
	class PathWrapper;
	PathWrapper* createPathWrapper(const panda::types::Path& path);
	const panda::types::Path& getPath(PathWrapper*);
}
