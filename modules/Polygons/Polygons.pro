include(../module.pri)

HEADERS       = \
	libs\clip2tri\clip2tri.h \
	libs\clipper\clipper.hpp \
	libs\poly2tri\poly2tri.h \
	libs\poly2tri\common\shapes.h \
	libs\poly2tri\common\utils.h \
	libs\poly2tri\sweep\advancing_front.h \
	libs\poly2tri\sweep\cdt.h \
	libs\poly2tri\sweep\sweep.h \
	libs\poly2tri\sweep\sweep_context.h
SOURCES       = \
	ComposePolygon.cpp \
	PolygonOperations.cpp \
	polygonsModule.cpp \
	SimplePolygons.cpp \
	Triangulation.cpp \
	libs\clip2tri\clip2tri.cpp \
	libs\clipper\clipper.cpp \
	libs\poly2tri\common\shapes.cc \
	libs\poly2tri\sweep\advancing_front.cc \
	libs\poly2tri\sweep\cdt.cc \
	libs\poly2tri\sweep\sweep.cc \
	libs\poly2tri\sweep\sweep_context.cc
