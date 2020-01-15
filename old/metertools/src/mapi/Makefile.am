# Makefile.am

MAINTAINERCLEANFILES = Makefile.in

# Meter library
lib_LTLIBRARIES = libmeter.la
libmeter_la_SOURCES = \
   meter.c meter.h \
   netmeter.c netmeter.h \
   fluke_189.c fluke_189.h \
   protek_506.c protek_506.h \
   rs_22812.c rs_22812.h \
   watts_up.c watts_up.h
libmeter_la_LDFLAGS = -version-info 1:0:0

# Include headers for install
include_HEADERS = \
   meter.h netmeter.h \
   watts_up.h fluke_189.h protek_506.h rs_22812.h

INCLUDES = -I../support
#CFLAGS = -g

