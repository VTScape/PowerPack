#!/bin/sh

# Primary build script for doing localized configuration
# $Id: autogen.sh,v 1.1.1.1 2007/02/07 15:07:22 fengx Exp $

PROG=`basename $0`
DIRNAME=`dirname $PROG`
if [ $DIRNAME = "." ]; then
   DIRNAME=`pwd`
fi

# main options, look here first if something goes wrong...
INTLTOOLIZE_FLAGS=--copy
GETTEXTIZE_FLAGS=--no-changelog
AUTOPOINT_FLAGS=
LIBTOOLIZE_FLAGS=--copy
ACLOCAL_FLAGS="-I $DIRNAME/m4"
#AUTOHEADER_FLAGS=-Wall
AUTOMAKE_FLAGS='--add-missing --copy'
#AUTOCONF_FLAGS=-Wall
# end main configuration options

DIE=0

fatal() {
   echo "$PROG: exited by previous error(s), return code was $1" >&2
   exit 1
}

(autoconf --version) < /dev/null > /dev/null 2>&1 || {
   echo "ERROR: You must have autoconf installed in order to compile."
   DIE=1
}

(grep "^A[CM]_PROG_LIBTOOL" $DIRNAME/configure.ac >/dev/null) && {
   (libtool --version) < /dev/null > /dev/null 2>&1 || {
      echo "ERROR: You must have libtool installed in order to compile."
      DIE=1
   }
}

(automake --version) < /dev/null > /dev/null 2>&1 || {
   echo "ERROR: You must have automake installed in order to compile."
   NO_AUTOMAKE=yes
   DIE=1
}

grep "^AM_GNU_GETTEXT" $DIRNAME/configure.ac >/dev/null && {
   grep "sed.*POTFILES" $DIRNAME/configure.ac >/dev/null || \
   (gettext --version) < /dev/null > /dev/null 2>&1 || {
      echo "ERROR: You must have gettext installed in order to compile."
      DIE=1
   }
}

test -n "$NO_AUTOMAKE" || (aclocal --version) < /dev/null > /dev/null 2>&1 || {
  echo "ERROR: You must have aclocal in order to compile."
  DIE=1
}

if test "$DIE" -eq 1; then
   echo "missing tools can be obtained at ftp://ftp.gnu.org/pub/gnu/"
   fatal 1
fi

for config in `find $DIRNAME -name configure.ac -print`; do
   dir=`dirname $config`
   if [ $dir = "." ]; then
      dir=`pwd`
   fi
   if test -f $dir/NO-AUTO-GEN; then
      echo "skipping $dir -- flagged with NO-AUTO-GEN"
   else
      echo "Processing $dir"
      cd $dir
      if grep "^AM_GNU_GETTEXT" configure.ac >/dev/null; then
         echo "Creating $dir/aclocal.m4 ..."
         test -r $dir/aclocal.m4 || touch $dir/aclocal.m4
         echo "Running gettextize...  Ignore non-fatal messages."
         echo "no" | gettextize ${GETTEXTIZE_FLAGS}
         echo "Making $dir/aclocal.m4 writable ..."
         test -r $dir/aclocal.m4 && chmod u+w $dir/aclocal.m4
      fi
      if grep "^AC_PROG_INTLTOOL" configure.ac >/dev/null; then
         echo "Running intltoolize ${INTLTOOLIZE_FLAGS}..."
         intltoolize ${INTLTOOLIZE_FLAGS} || fatal 1
      fi
      if grep "^AM_GNU_GETTEXT_VERSION" configure.ac > /dev/null; then
         echo "Running autopoint ${AUTOPOINT_FLAGS}..."
         autopoint ${AUTOPOINT_FLAGS}
      fi
      if grep "^AM_PROG_XML_I18N_TOOLS" configure.ac >/dev/null; then
         echo "Running xml-i18n-toolize..."
         xml-i18n-toolize --copy --force --automake
      fi
      if grep "^A[MC]_PROG_LIBTOOL" configure.ac >/dev/null; then
         if test -z "$NO_LIBTOOLIZE" ; then
            echo "Running libtoolize ${LIBTOOLIZE_FLAGS}..."
            libtoolize ${LIBTOOLIZE_FLAGS} || fatal 1
         fi
      fi
      echo "Running aclocal ${ACLOCAL_FLAGS}..."
      aclocal ${ACLOCAL_FLAGS} || fatal 1
      if grep "^AM_CONFIG_HEADER" configure.ac >/dev/null; then
         echo "Running autoheader..."
         autoheader ${AUTOHEADER_FLAGS} || fatal 1
      fi
      automake ${AUTOMAKE_FLAGS} || fatal 1
      autoconf ${AUTOCONF_FLAGS} || fatal 1
   fi
done

if [ -d $DIRNAME/autom4te.cache ]; then
   rm -rf $DIRNAME/autom4te.cache
fi

