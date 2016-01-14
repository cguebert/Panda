#include <panda/object/ObjectFactory.h>

panda::ModuleHandle coreModule = panda::RegisterModule("core")
		.setDescription("Core components")
		.setLicense("GPL")
		.setVersion("1.0");
