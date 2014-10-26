#include <panda/ObjectFactory.h>

panda::ModuleHandle listModule = panda::RegisterModule("List")
		.setDescription("Generic objects that accept multiple types, and manipulate lists.")
		.setLicense("GPL")
		.setVersion("1.0");
