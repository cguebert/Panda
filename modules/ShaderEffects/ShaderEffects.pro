include(../module.pri)

HEADERS       = \
	ShaderEffects.h
SOURCES       = \
	Enblend.cpp \
	GaussianBlur.cpp \
	GaussianPyramid.cpp \
	MergeImages.cpp \
	SimpleResize.cpp \
	ShaderEffects.cpp \
	TextureProduct.cpp

RESOURCES     = ShaderEffects.qrc
