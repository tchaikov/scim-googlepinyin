AC_INIT(android, 0.1, tchaikov@gmail.com)
AC_CANONICAL_SYSTEM()
AC_PREFIX_DEFAULT([/usr])
AC_CONFIG_SRCDIR(src/google_imengine.cpp)
AM_CONFIG_HEADER(config.h)

GETTEXT_PACKAGE=scim-googlepinyin
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [The gettext domain])

dnl Init gettext
ALL_LINGUAS="zh_CN"
AM_GLIB_GNU_GETTEXT

LTLIBINTL=
AC_SUBST(LTLIBINTL)

AC_LANG(C++)

dnl-----------------------------------------------------------------------
dnl    --enable-debug
dnl-----------------------------------------------------------------------
AC_ARG_ENABLE([debug],
    AC_HELP_STRING([--enable-debug], [compile in debug mode (default no) ]),
    [DEBUGMODE="${enableval}"],
    [DEBUGMODE="no"])

if test "X$DEBUGMODE" = "Xyes" ; then
    AC_DEFINE(DEBUG, 1, [Define if you want to make for debugging.])
fi

AC_ARG_WITH([google-pinyin-datadir],
    AC_HELP_STRING([--with-android-datadir=PATH],
                   [specify the directory where the lexicon data is installed]),
    [SCIM_GOOGLEPINYIN_DATADIR="${withval}"],
    [SCIM_GOOGLEPINYIN_DATADIR=""])
AC_SUBST(SCIM_GOOGLEPINYIN_DATADIR)

# Check SCIM
SCIM_VERSION=1.4.0
AC_SUBST(SCIM_VERSION)
export PKG_CONFIG_PATH="/opt/scim/lib/pkgconfig:${PKG_CONFIG_PATH}"
PKG_CHECK_MODULES(SCIM,[scim >= $SCIM_VERSION])

# Check if we should build setup module
PKG_CHECK_MODULES(SCIM_GTKUTILS,[scim-gtkutils >= $SCIM_VERSION],
                                [SCIM_HAS_GTKUTILS=yes],
                                [SCIM_HAS_GTKUTILS=no])
    
# Set paths
SCIM_ICONDIR=`$PKG_CONFIG --variable=icondir scim`
SCIM_MODULEDIR=`$PKG_CONFIG --variable=moduledir scim`
SCIM_DATADIR=`$PKG_CONFIG --variable=scimdatadir scim`
SCIM_GOOGLEPINYIN_DATADIR=$SCIM_DATADIR/googlepinyin
AC_SUBST(SCIM_ICONDIR)
AC_SUBST(SCIM_MODULEDIR)
AC_SUBST(SCIM_DATADIR)
AC_SUBST(SCIM_GOOGLEPINYIN_DATADIR)

AM_CONDITIONAL(SCIM_BUILD_SETUP, [test "X$SCIM_HAS_GTKUTILS" = "Xyes"])

AM_INIT_AUTOMAKE

# Check for programs
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LIBTOOL

# Checks for libraries.

# Checks for header files.
AC_PATH_X
AC_HEADER_STDC
AC_CHECK_HEADERS(assert.h iconv.h fcntl.h locale.h stdlib.h string.h unistd.h libintl.h)

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MMAP
AC_CHECK_FUNCS(memset munmap setlocale strdup log2 exp2)

AC_CONFIG_FILES([Makefile 
                 data/Makefile
                 src/Makefile
                  src/include/Makefile
                 src/data/Makefile
                 src/share/Makefile
                 src/command/Makefile])
AC_OUTPUT

