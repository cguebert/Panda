TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS = main \
list \
particles

main.file = MainModules.pro
list.file = list/List.pro
particles.file = particles/Particles.pro
