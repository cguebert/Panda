TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS = core \
          app \
		  modules
		  
app.file = PandaGUI.pro
app.depends = core

modules.file = modules/PandaModules.pro
modules.depends = core

core.file = core/PandaCore.pro
