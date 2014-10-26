#include <panda/ObjectFactory.h>

panda::ModuleHandle mainModule = panda::RegisterModule("main")
		.setDescription("Main modules for components not yet sorted by their type")
		.setLicense("GPL")
		.setVersion("1.0");
