dnl 
dnl    configure.ac
dnl 
AC_PREREQ(2.57)
AC_COPYRIGHT([Copyright (C) Mark Femal])
AC_INIT([Meter Tools],[1.0.0],[mfemal@gmail.com])
AC_CONFIG_SRCDIR(src)
AM_INIT_AUTOMAKE([1.9 foreign])
AC_CANONICAL_HOST

#---------------------------------------------------------
#	C language relative
AC_LANG(C)
AC_PROG_CC
AC_PROG_CPP
AC_PROG_LIBTOOL
AC_PROG_MAKE_SET
AC_C_INLINE

AC_MSG_CHECKING([for ISO C99 varargs macros in C])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
        [[
            int a(int, int, int);
#           define call_a(...) a(1,__VA_ARGS__)
        ]],
        [[ call_a(2,3); ]])
    ],  dnl AC_LANG_PROGRAM() end
    [
        # action if-found
        AC_MSG_RESULT([yes])
        AC_DEFINE([HAVE_ISO_VARARGS], 1,
                  [Define if you have ISO C99 vararg macros])
    ],
    [
        # action if-not-found
        AC_MSG_RESULT([no])
    ])

# Libc features
AC_C_CONST
AC_TYPE_UID_T
AC_TYPE_PID_T
AC_STRUCT_TM
AC_FUNC_MEMCMP
AC_TYPE_SIGNAL
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(string.h)
AC_CHECK_HEADERS(math.h)
AC_CHECK_HEADERS(sys/time.h)
AC_CHECK_HEADERS(syslog.h)
AC_CHECK_HEADERS(sys/sched.h sched.h)
AC_HEADER_TIME
AC_CHECK_FUNCS([memcpy memset])
AC_CHECK_FUNCS([lockf flock])
AC_CHECK_FUNCS(nanosleep,,
   [AC_CHECK_LIB(rt,nanosleep,LIBS="$LIBS -lrt",
      [AC_CHECK_LIB(posix4,nanosleep,LIB="$LIBS -lposix4")]
)])
AC_MSG_CHECKING([whether stdio.h declares vsnprintf])
AC_EGREP_HEADER(vsnprintf, stdio.h,
   [AC_DEFINE(STDIO_H_DECLARES_VSNPRINTF, 1,
             [ If <stdio.h> declares vsnprintf.  ])
    AC_MSG_RESULT(yes)],
    AC_MSG_RESULT(no))

#---------------------------------------------------------
# Network related libraries
AC_CHECK_HEADERS([sys/types.h sys/socket.h netinet/in.h arpa/inet.h])

GET_NETLIBS(NETLIBS, [success=yes], [success=no])
if test x"$success" != xyes
then
    AC_MSG_FAILURE([required libraries are not found or not usable])
fi
AC_SUBST(NETLIBS)

#---------------------------------------------------------
# Change the host name the client tools connect to
default_meter_host=localhost
AC_ARG_WITH(meter_host, [AC_HELP_STRING([--with-meter-host=name],
   [Set the default meter host, default is localhost])],
   [meter_host="$withval"],
   [meter_host="$default_meter_host"])
AC_MSG_CHECKING([for meter host])
AC_MSG_RESULT([$meter_host])
AC_SUBST(meter_host)
AC_DEFINE_UNQUOTED([METER_HOST], ["$meter_host"],
   [Define for the default meter host for connections])

#---------------------------------------------------------
# Change the port the tools listen or connect to
default_meter_tcp_port=6999
AC_ARG_WITH(meter_tcp_port, [AC_HELP_STRING([--with-tcp-port=number],
   [Set the default meter TCP port, default is 6999])],
   [meter_tcp_port="$withval"],
   [meter_tcp_port="$default_meter_tcp_port"])
AC_MSG_CHECKING([for meter TCP port])
AC_MSG_RESULT([$meter_tcp_port])
AC_SUBST(meter_tcp_port)
AC_DEFINE_UNQUOTED([METER_TCP_PORT], [$meter_tcp_port],
   [Define for meter TCP port for the server])

#---------------------------------------------------------
# Change the broadcast UDP port or the PEP TCP port
default_serial_port=/dev/ttyS0
AC_ARG_WITH(serial_port, [AC_HELP_STRING([--with-serial-port=dev],
   [Set the default serial port, default is /dev/ttyS0])],
   [serial_port="$withval"],
   [serial_port="$default_serial_port"])
AC_MSG_CHECKING([for serial port])
AC_MSG_RESULT([$serial_port])
AC_SUBST(serial_port)
AC_DEFINE_UNQUOTED([SERIAL_PORT], ["$serial_port"],
   [Define for the default serial port])

# Documentation generation tools
### AC_CHECK_PROG([HAVE_DOXYGEN], [doxygen], [yes], [no])
### AC_CHECK_PROG([HAVE_DOT], [dot], [yes], [no])
### AM_CONDITIONAL([HAVE_DOXYGEN], [ test x"$HAVE_DOXYGEN" = xyes ])

AM_CONFIG_HEADER(config.h)
AH_TOP([
#ifndef _CONFIG_H
#define _CONFIG_H
])
AH_BOTTOM([
#endif
])

AC_SUBST(sbindir)
AC_CONFIG_FILES([Makefile m4/Makefile src/Makefile src/mapi/Makefile src/support/Makefile src/tools/Makefile docs/Makefile])
AC_OUTPUT()

