TEMPLATE = lib
CONFIG += dll c++11
TARGET = Particles
HEADERS       = \
	Particle.h \
	ParticleEngine.h \
	ParticleEffector.h
SOURCES       = \
	ParticleCollision.cpp \
	ParticleCreation.cpp \
	ParticleDestruction.cpp \
	ParticleEngine.cpp \
	ParticleForceField.cpp
INCLUDEPATH = .
QT += widgets xml opengl
CONFIG += c++11
DEFINES +=	PANDA_LOG_EVENTS \
			_CRT_SECURE_NO_WARNINGS \
			_SCL_SECURE_NO_WARNINGS

ROOT_DIR = ../..
EXE_PATH = $${ROOT_DIR}/bin

Release:DESTDIR = $${EXE_PATH}/modules
Debug:DESTDIR = $${EXE_PATH}/modules_d
Release:LIBS += $${EXE_PATH}/PandaCore.lib
Debug:LIBS += $${EXE_PATH}/PandaCore_d.lib

PRECOMPILED_HEADER = $${ROOT_DIR}/core/panda/pch.h

#DEFINES += PANDA_DOUBLE

BOOST = $${ROOT_DIR}/../boost_1_56_0

INCLUDEPATH += $${BOOST}/
INCLUDEPATH += $${ROOT_DIR}
INCLUDEPATH += $${ROOT_DIR}/core
LIBPATH += $${BOOST}/stage/lib
