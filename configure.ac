#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
AC_MSG_NOTICE([Hinweis])
AC_MSG_WARN([Warnung])
AC_MSG_ERROR([Fehler])

AC_PREREQ([2.64])
AC_INIT(GarminPlugin, 0.2, andreas.diesner@gmx.de)
AC_CONFIG_SRCDIR(main.cpp)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.

PKG_PROG_PKG_CONFIG()

#
# XULRunner check

PKG_CHECK_MODULES(XULRUNNER, [libxul < 1.9.2],[ AC_MSG_NOTICE([Using libxul < 1.9.2])
    ],[AC_MSG_WARN([libxul < 1.9.2 not found, trying another])])

if test "$XULRUNNER_CFLAGS" = ""; then
PKG_CHECK_MODULES(XULRUNNER, [libxul >= 1.9.2],[ AC_DEFINE([HAVE_NEW_XULRUNNER],[1],[Define to 1 if using libxul 1.9.2 or higher]) AC_MSG_NOTICE([Using libxul >= 1.9.2])
    ],[AC_MSG_NOTICE([libxul >= 1.9.2 not found, trying another])])
fi

if test "$XULRUNNER_CFLAGS" = ""; then
  xullibname=no
  PKG_CHECK_MODULES(XULRUNNER, [libxul-embedding-unstable],
		     [xullibname=libxul-embedding-unstable],
      [PKG_CHECK_MODULES(XULRUNNER, [libxul-embedding],
		         [xullibname=libxul-embedding],
        [PKG_CHECK_MODULES(XULRUNNER, [xulrunner-gtkmozembed],
			  [xullibname=xulrunner-gtkmozembed],
  	  [PKG_CHECK_MODULES(XULRUNNER, [firefox2-gtkmozembed],
			    [xullibname=firefox2-gtkmozembed],
  	    [PKG_CHECK_MODULES(XULRUNNER, [firefox-gtkmozembed],
		  	      [xullibname=firefox-gtkmozembed],
  	      [xullibname=no])])])])])

  if test x$xullibname = xno; then
	  AC_MSG_ERROR([Unable to find xulrunner. Try sudo apt-get install xulrunner-dev.])
  fi

fi 

AC_SUBST(XULRUNNER_CFLAGS)
AC_SUBST(XULRUNNER_LIBS)


#
# ZLIB Check
#

#
# Copyright 2007 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# GGL_CHECK_ZLIB([MINIMUM-VERSION],
#                [ACTION-IF-FOUND],
#                [ACTION-IF-NOT-FOUND])
# Check for zlib library, if zlib library with version greater than
# MINIMUM-VERSION is found, ACTION-IF-FOUND will be executed,
# and ZLIB_CPPFLAGS, ZLIB_LIBS, ZLIB_LDFLAGS
# will be set accordingly,
# otherwise, ACTION-IF-NOT-FOUND will be executed.


AC_DEFUN([GGL_CHECK_ZLIB], [
AC_ARG_WITH([zlib-libdir],
	    AS_HELP_STRING([--with-zlib-libdir=DIR],
		[specify where to find zlib library]),
	    [zlib_libdir=$withval],
	    [zlib_libdir=""])

AC_ARG_WITH([zlib-incdir],
	    AS_HELP_STRING([--with-zlib-incdir=DIR],
		[specify where to find zlib header files]),
	    [zlib_incdir=$withval],
	    [zlib_incdir=""])

min_zlib_version=ifelse([$1], , 1.2.3, [$1])

AC_MSG_CHECKING([for zlib version >= $min_zlib_version])

zlib_save_CPPFLAGS="$CPPFLAGS"
zlib_save_LIBS="$LIBS"
zlib_save_LDFLAGS="$LDFLAGS"

ZLIB_CPPFLAGS=""
ZLIB_LDFLAGS=""
ZLIB_LIBS="-lz"

if test "x$zlib_incdir" != "x" ; then
  ZLIB_CPPFLAGS="$ZLIB_CPPFLAGS -I$zlib_incdir"
elif test -f "$prefix/include/zlib.h" ; then
  ZLIB_CPPFLAGS="$ZLIB_CPPFLAGS -I$prefix/include"
elif test -f "/usr/include/zlib.h" ; then
  ZLIB_CPPFLAGS="$ZLIB_CPPFLAGS -I/usr/include"
elif test -f "/usr/local/include/zlib.h" ; then
  ZLIB_CPPFLAGS="$ZLIB_CPPFLAGS -I/usr/local/include"
elif test -f "/opt/local/include/zlib.h" ; then
  ZLIB_CPPFLAGS="$ZLIB_CPPFLAGS -I/opt/local/include"
fi

if test "x$zlib_libdir" != "x" ; then
  ZLIB_LDFLAGS="$ZLIB_LDFLAGS -L$zlib_libdir"
fi

CPPFLAGS="$ZLIB_CPPFLAGS $CPPFLAGS"
LDFLAGS="$ZLIB_LDFLAGS $LDFLAGS"
LIBS="$ZLIB_LIBS $LIBS"

min_zlib_vernum=`echo $min_zlib_version | sed 's/\.//g'`

AC_LINK_IFELSE([[
#include<zlib.h>

#if ZLIB_VERNUM < 0x${min_zlib_vernum}0
#error "zlib version is too low."
#endif

int main() {
  z_stream strm;
  deflate(&strm, 0);
  return 0;
}
]],
[ac_have_zlib=yes],
[ac_have_zlib=no])

if test "x$ac_have_zlib" = "xyes" ; then
  ZLIB_CPPFLAGS="$ZLIB_CPPFLAGS"
  ZLIB_LIBS="$ZLIB_LIBS"
  ZLIB_LDFLAGS="$ZLIB_LDFLAGS"
  AC_SUBST(ZLIB_CPPFLAGS)
  AC_SUBST(ZLIB_LIBS)
  AC_SUBST(ZLIB_LDFLAGS)
  AC_MSG_RESULT([yes (CPPFLAGS=$ZLIB_CPPFLAGS  LIBS=$ZLIB_LIBS  LDFLAGS=$ZLIB_LDFLAGS)])
  ifelse([$2], , :, [$2])
else
  AC_MSG_RESULT([no])
  ifelse([$3], , :, [$3])
fi

CPPFLAGS=$zlib_save_CPPFLAGS
LIBS=$zlib_save_LIBS
LDFLAGS=$zlib_save_LDFLAGS
])


has_zlib=no
GGL_CHECK_ZLIB([1.2.3], [has_zlib=yes], [has_zlib=no])

if test x$has_zlib != xyes; then
  AC_MSG_ERROR([Library zlib is not available. Try sudo apt-get install zlib1g-dev])
fi


#
# Check for ticpp
#

AC_MSG_CHECKING([for ticpp])

AC_ARG_WITH([ticpp-libdir],
	    AS_HELP_STRING([--with-ticpp-libdir=DIR],
		[specify where to find ticpp library (libticpp.a)]),
	    [ticpp_libdir=$withval],
	    [ticpp_libdir=""])

AC_ARG_WITH([ticpp-incdir],
	    AS_HELP_STRING([--with-ticpp-incdir=DIR],
		[specify where to find ticpp header files (ticpp.h)]),
	    [ticpp_incdir=$withval],
	    [ticpp_incdir=""])

if test "x$ticpp_incdir" != "x" ; then
  TICPP_CPPFLAGS="-I$ticpp_incdir"
elif test -f "../ticpp/ticpp.h" ; then
  TICPP_CPPFLAGS="-I../ticpp/"
elif test -f "../ticpp-read-only/ticpp.h" ; then
  TICPP_CPPFLAGS="-I../ticpp-read-only"
elif test -f "ticpp-read-only/ticpp.h" ; then
  TICPP_CPPFLAGS="-Iticpp-read-only"
elif test -f "ticpp/ticpp.h" ; then
  TICPP_CPPFLAGS="-Iticpp"
else
  AC_MSG_RESULT([no])
  AC_MSG_ERROR([Unable to find ticpp header files. Try 'svn export http://ticpp.googlecode.com/svn/trunk/ ticpp' or use --with-ticpp-incdir=DIR])
fi

if test "x$ticpp_libdir" != "x" ; then
  if test -f "$ticpp_libdir/libticpp.a" ; then
    TICPP_LDFLAGS="$ticpp_libdir/libticpp.a"
  else
    AC_MSG_RESULT([no])
    AC_MSG_ERROR([Unable to find libticpp.a in $ticpp_libdir. Is ticpp compiled?])
  fi
elif test -f "../ticpp/lib/libticpp.a" ; then
  TICPP_LDFLAGS="../ticpp/lib/libticpp.a"
elif test -f "../ticpp-read-only/lib/libticpp.a" ; then
  TICPP_LDFLAGS="../ticpp-read-only/lib/libticpp.a"
elif test -f "ticpp-read-only/lib/libticpp.a" ; then
  TICPP_LDFLAGS="ticpp-read-only/lib/libticpp.a"
elif test -f "ticpp/lib/libticpp.a" ; then
  TICPP_LDFLAGS="ticpp/lib/libticpp.a"
else
  AC_MSG_RESULT([no])
  AC_MSG_ERROR([Unable to find libticpp.a. Try 'svn export http://ticpp.googlecode.com/svn/trunk/ ticpp' and compile source or use --with-ticpp-libdir=DIR])
fi

AC_SUBST(TICPP_CPPFLAGS)
AC_SUBST(TICPP_LDFLAGS)

AC_MSG_RESULT([yes])

#
# Check GarminTools
#

AC_MSG_CHECKING([for garmintools])


AC_ARG_WITH([garmintools-libdir],
	    AS_HELP_STRING([--with-garmintools-libdir=DIR],
		[specify where to find garmintools library (libgarmintools.a)]),
	    [garmintools_libdir=$withval],
	    [garmintools_libdir=""])

AC_ARG_WITH([garmintools-incdir],
	    AS_HELP_STRING([--with-garmintools-incdir=DIR],
		[specify where to find garmintools header files (garmin.h)]),
	    [garmintools_incdir=$withval],
	    [garmintools_incdir=""])

if test "x$garmintools_incdir" != "x" ; then
  GARMINTOOLS_CPPFLAGS="-I$garmintools_incdir"
elif test -f "../garmintools/src/garmin.h" ; then
  GARMINTOOLS_CPPFLAGS="-I../garmintools/src/"
elif test -f "../garmintools-read-only/src/garmin.h" ; then
  GARMINTOOLS_CPPFLAGS="-I../garmintools-read-only/src"
elif test -f "garmintools-read-only/src/garmin.h" ; then
  GARMINTOOLS_CPPFLAGS="-Igarmintools-read-only/src"
elif test -f "garmintools/src/garmin.h" ; then
  GARMINTOOLS_CPPFLAGS="-Igarmintools/src"
else
  AC_MSG_RESULT([no])
  AC_MSG_ERROR([Unable to find garmintools header files. Try 'svn export http://garmintools.googlecode.com/svn/trunk/ garmintools' or use --with-garmintools-incdir=DIR])
fi

if test "x$garmintools_libdir" != "x" ; then
  if test -f "$garmintools_libdir/libgarmintools.a" ; then
    GARMINTOOLS_LDFLAGS="$garmintools_libdir/libgarmintools.a"
  else
    AC_MSG_RESULT([no])
    AC_MSG_ERROR([Unable to find libgarmintools.a in $garmintools_libdir. Is garmintools compiled?])
  fi
elif test -f "../garmintools/src/.libs/libgarmintools.a" ; then
  GARMINTOOLS_LDFLAGS="../garmintools/src/.libs/libgarmintools.a"
elif test -f "garmintools/src/.libs/libgarmintools.a" ; then
  GARMINTOOLS_LDFLAGS="garmintools/src/.libs/libgarmintools.a"
elif test -f "../garmintools-read-only/src/.libs/libgarmintools.a" ; then
  GARMINTOOLS_LDFLAGS="../garmintools-read-only/src/.libs/libgarmintools.a"
elif test -f "garmintools-read-only/src/.libs/libgarmintools.a" ; then
  GARMINTOOLS_LDFLAGS="garmintools-read-only/src/.libs/libgarmintools.a"
elif test -f "garmintools-read-only/libgarmintools.a" ; then
  GARMINTOOLS_LDFLAGS="garmintools-read-only/libgarmintools.a"
elif test -f "../garmintools-read-only/libgarmintools.a" ; then
  GARMINTOOLS_LDFLAGS="../garmintools-read-only/libgarmintools.a"
elif test -f "garmintools/libgarmintools.a" ; then
  GARMINTOOLS_LDFLAGS="garmintools/libgarmintools.a"
elif test -f "../garmintools/libgarmintools.a" ; then
  GARMINTOOLS_LDFLAGS="../garmintools/libgarmintools.a"
else
  AC_MSG_RESULT([no])
  AC_MSG_ERROR([Unable to find libgarmintools.a. Try 'svn export http://garmintools.googlecode.com/svn/trunk/ garmintools' and compile source or use --with-garmintools-libdir=DIR])
fi


AC_SUBST(GARMINTOOLS_CPPFLAGS)
AC_SUBST(GARMINTOOLS_LDFLAGS)

AC_MSG_RESULT([yes])


# Checks for header files.
AC_CHECK_HEADERS([mntent.h stdlib.h unistd.h])

xulrunner_save_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $XULRUNNER_CFLAGS"
XULRUNNER_REQUIRED_HEADERS=1
AC_CHECK_HEADERS(npapi.h,[],
	AC_MSG_ERROR([Unable to find xulrunner (missing npapi.h). Try sudo apt-get install xulrunner-dev.]))
AC_CHECK_HEADERS(npruntime.h,,
	AC_MSG_ERROR([Unable to find xulrunner (missing npruntime.h). Try sudo apt-get install xulrunner-dev.]), 
        [#include <npapi.h>])

AC_CHECK_HEADERS([zipstub.h],[],[],[])
AC_CHECK_HEADERS([prtypes.h],[],[],[])
AC_CHECK_HEADERS([npfunctions.h],[],[],[])
CPPFLAGS="$xulrunner_save_CPPFLAGS"

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T

# Checks for library functions.
AC_FUNC_GETMNTENT
AC_CHECK_FUNCS([getmntent localtime_r memmove mkdir])

AC_CONFIG_HEADERS([config.h])


AC_CONFIG_FILES([Makefile])
AC_OUTPUT
