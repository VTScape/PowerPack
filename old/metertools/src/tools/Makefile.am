# Makefile.am

MAINTAINERCLEANFILES = Makefile.in

# Programs
bin_PROGRAMS = mclient mlogger msend

mclient_SOURCES = mclient.c
mclient_LDADD = ../mapi/libmeter.la ../support/libsupport.la

# General testing utilities
msend_SOURCES = msend.c
msend_LDADD = ../mapi/libmeter.la ../support/libsupport.la

mlogger_SOURCES = mlogger.c
mlogger_LDADD = -lpthread ../mapi/libmeter.la ../support/libsupport.la

INCLUDES = -I../mapi -I../support

