#
#   GET_NETLIBS([libs_out], [action-if-found], [action-if-not-found])
#
#   Search for additional libraries containing inet_ntoa() and socket().
#
#   Typically, these are -lnsl and -lsocket for Solaris and none for
#   Linux.
#
AC_DEFUN([GET_NETLIBS], [
    AS_VAR_PUSHDEF([save_LIBS], [netlibs_save_LIBS])
    AS_VAR_PUSHDEF([netlibs], [netlibs_netlibs])
    AS_VAR_PUSHDEF([success], [netlibs_success])

    save_LIBS="$LIBS"
    LIBS=''
    success=yes

    AC_SEARCH_LIBS([inet_ntoa], [nsl], [success=yes], [success=no])
    if test x"$success" = xyes
    then
        AC_SEARCH_LIBS([socket], [socket], [success=yes], [success=no])
    fi

    netlibs="$LIBS"
    LIBS="$save_LIBS"

    AS_VAR_POPDEF([save_LIBS])
    AS_VAR_POPDEF([netlibs])
    AS_VAR_POPDEF([success])

    if test x"$netlibs_success" = xyes
    then
        ifelse([$1], [], [:], [$1="$netlibs_netlibs"])
        ifelse([$2], [], [:], [$2])
    else
        ifelse([$3], [], [:], [$3])
    fi
])
