TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS = core \
          app
		  
app.file = pandaGUI.pro
app.depends = core

core.file = core/pandaCore.pro
