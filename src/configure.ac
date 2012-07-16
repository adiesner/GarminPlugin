#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
AC_MSG_NOTICE([Hinweis])
AC_MSG_WARN([Warnung])
AC_MSG_ERROR([Fehler])

AC_PREREQ([2.64])
AC_INIT(GarminPlugin, 0.2, garminplugin@andreas-diesner.de)
AC_CONFIG_SRCDIR(main.cpp)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.

PKG_PROG_PKG_CONFIG()

PKG_CHECK_MODULES([LIBUSB], [libusb])
AC_SUBST(LIBUSB_CFLAGS)
AC_SUBST(LIBUSB_LIBS)

PKG_CHECK_MODULES([ZLIB], [zlib])
AC_SUBST(ZLIB_CFLAGS)
AC_SUBST(ZLIB_LIBS)

#
# Check for GNU Crypt library
#
AC_ARG_WITH(
	[libgcrypt-prefix],
	[AC_HELP_STRING([--with-libgcrypt-prefix=DIR], [define libgcrypt prefix, default /usr])],
	,
	[with_libgcrypt_prefix="/usr" ]
)

AC_MSG_CHECKING([for libgcrypt])
if ! test -x "${with_libgcrypt_prefix}/bin/libgcrypt-config"; then
	AC_MSG_ERROR([Cannot locate libgcrypt])
else
	AC_MSG_RESULT([found])
	GCRYPT_CFLAGS=`"${with_libgcrypt_prefix}/bin/libgcrypt-config" --cflags`
	GCRYPT_LIBS=`"${with_libgcrypt_prefix}/bin/libgcrypt-config" --libs`
	AC_SUBST(GCRYPT_CFLAGS)
	AC_SUBST(GCRYPT_LIBS)
fi


#
# Check for tinyxml
#

AC_MSG_CHECKING([for tinyxml])

AC_ARG_WITH([tinyxml-libdir],
	    AS_HELP_STRING([--with-tinyxml-libdir=DIR],
		[specify where to find tinyxml library (libtinyxml.so)]),
	    [tinyxml_libdir=$withval],
	    [tinyxml_libdir=""])

AC_ARG_WITH([tinyxml-incdir],
	    AS_HELP_STRING([--with-tinyxml-incdir=DIR],
		[specify where to find tinyxml header files (tinyxml.h)]),
	    [tinyxml_incdir=$withval],
	    [tinyxml_incdir=""])

if test "x$tinyxml_incdir" != "x" ; then
  TINYXML_CPPFLAGS="-I$tinyxml_incdir"
elif test -f "../tinyxml/tinyxml.h" ; then
  TINYXML_CPPFLAGS="-I../tinyxml/"
elif test -f "/usr/include/tinyxml.h" ; then
  TINYXML_CPPFLAGS="-I/usr/include"
elif test -f "tinyxml/tinyxml.h" ; then
  TINYXML_CPPFLAGS="-Itinyxml"
else
  AC_MSG_RESULT([no])
  AC_MSG_ERROR([Unable to find tinyxml header file (tinyxml.h). Try 'apt-get install tinyxml-dev' or use --with-tinyxml-incdir=DIR])
fi

if test "x$tinyxml_libdir" != "x" ; then
  if test -f "$tinyxml_libdir/libtinyxml.so" ; then
    TINYXML_LDFLAGS="$tinyxml_libdir/libtinyxml.so"
  else
    AC_MSG_RESULT([no])
    AC_MSG_ERROR([Unable to find libtinyxml.so in $tinyxml_libdir. Is tinyxml compiled?])
  fi
elif test -f "/usr/lib64/libtinyxml.so" ; then
  TINYXML_LDFLAGS="/usr/lib64/libtinyxml.so"
elif test -f "/usr/lib/libtinyxml.so" ; then
  TINYXML_LDFLAGS="/usr/lib/libtinyxml.so"
elif test -f "../tinyxml/libtinyxml.so" ; then
  TINYXML_LDFLAGS="../tinyxml/libtinyxml.so"
elif test -f "../tinyxml/tinyxml.o" ; then
  TINYXML_LDFLAGS="../tinyxml/tinyxml.o ../tinyxml/tinystr.o ../tinyxml/tinyxmlerror.o ../tinyxml/tinyxmlparser.o"
else
  AC_MSG_RESULT([no])
  AC_MSG_ERROR([Unable to find libtinyxml.so. Try 'apt-get install tinyxml' and compile source or use --with-tinyxml-libdir=DIR])
fi

AC_SUBST(TINYXML_CPPFLAGS)
AC_SUBST(TINYXML_LDFLAGS)

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
elif test -f "/usr/include/garmin.h" ; then
  GARMINTOOLS_CPPFLAGS="-I/usr/include/"
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
  elif test -f "$garmintools_libdir/libgarmintools.so" ; then
    GARMINTOOLS_LDFLAGS="$garmintools_libdir/libgarmintools.so"
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
elif test -f "/usr/lib/libgarmintools.so" ; then
  GARMINTOOLS_LDFLAGS="/usr/lib/libgarmintools.so"
elif test -f "/usr/lib64/libgarmintools.so" ; then
  GARMINTOOLS_LDFLAGS="/usr/lib64/libgarmintools.so"
else
  AC_MSG_RESULT([no])
  AC_MSG_ERROR([Unable to find libgarmintools.a. Try 'svn export http://garmintools.googlecode.com/svn/trunk/ garmintools' and compile source or use --with-garmintools-libdir=DIR])
fi


AC_SUBST(GARMINTOOLS_CPPFLAGS)
AC_SUBST(GARMINTOOLS_LDFLAGS)

AC_MSG_RESULT([yes])


# Checks for header files.
AC_CHECK_HEADERS([mntent.h stdlib.h unistd.h])

AC_CHECK_HEADERS([zipstub.h],[],[],[])
AC_CHECK_HEADERS([prtypes.h],[],[],[])

AC_MSG_RESULT([yes])


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
