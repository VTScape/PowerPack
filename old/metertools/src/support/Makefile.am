# Makefile.am

MAINTAINERCLEANFILES = Makefile.in

lib_LTLIBRARIES = libsupport.la

libsupport_la_SOURCES= \
   connectsock.c connectsock.h \
   passivesock.c passivesock.h \
   util.c util.h \
   stringbuffer.c stringbuffer.h

# Libtools versioning scheme is: current:revision:age
libsupport_la_LDFLAGS = -version-info 1:0:0

include_HEADERS = connectsock.h passivesock.h util.h stringbuffer.h

