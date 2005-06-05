# -----------------------------------------------------------------------------
# iXiON library extended compiler checks
# -----------------------------------------------------------------------------
# (c) iXiONmedia 1999
# -----------------------------------------------------------------------------



dnl IX_PATH_ANY_LIBRARY(
dnl  1: NAME,
dnl  2: LIBRARY,
dnl  3: EXTRA-LIBRARIES,
dnl  4: HEADER-NAME,
dnl  5: EXTRA-CFLAGS,
dnl  6: CHECK-FUNCTION
dnl  7: [,ACTION-IF-FOUND
dnl  8: [,ACTION-IF-NOT-FOUND]])
AC_DEFUN([IX_PATH_ANY_LIBRARY],[
  AC_ARG_WITH([$1],
    AC_HELP_STRING([--with-$1(=path)]
    [use $1 (installed to path)]),
    [if test "x$withval" = "xno"; then
       ix_$1_present="no"
     else
       ix_$1_present="yes"
       ix_$1_base="$withval"
       fi],
    [ix_$1_present="yes"
     ix_$1_base=""])

  AC_SUBST(translit($1,[a-z],[A-Z])_CFLAGS)
  AC_SUBST(translit($1,[a-z],[A-Z])_LDFLAGS)
  AC_SUBST(translit($1,[a-z],[A-Z])_LIBS)

  if test "x$ix_$1_present" = "xyes"; then
    if ! test "x$ix_$1_base" = "x"; then
      translit($1,[a-z],[A-Z])_CFLAGS="-I$ix_$1_base/include"
      translit($1,[a-z],[A-Z])_LDFLAGS="-L$ix_$1_base/lib"
    else
      translit($1,[a-z],[A-Z])_CFLAGS=""
      translit($1,[a-z],[A-Z])_LDFLAGS=""
      fi
    translit($1,[a-z],[A-Z])_LIBS="-l$2 $3"
    
  
    ac_backup_CPPFLAGS="$CPPFLAGS"
    ac_backup_LDFLAGS="$LDFLAGS"

    CPPFLAGS="$CPPFLAGS $m4_translit($1,[a-z],[A-Z])_CFLAGS $5"
    LDFLAGS="$LDFLAGS $m4_translit($1,[a-z],[A-Z])_LDFLAGS"
  
    AC_LANG_PUSH(C)
  
    AC_CHECK_HEADER($4,,
      [AC_MSG_ERROR([$1 headers not found.])])
  
    AC_CHECK_LIB($2,$6,,
      [AC_MSG_ERROR([$1 libraries not found.])],[$3])
  
    AC_LANG_POP

    CPPFLAGS="$ix_backup_CPPFLAGS"
    LDFLAGS="$ix_backup_LDFLAGS"
    
    $7
  else
    translit($1,[a-z],[A-Z])_CFLAGS=""
    translit($1,[a-z],[A-Z])_LDFLAGS=""
    translit($1,[a-z],[A-Z])_LIBS=""
    
    $8
    fi
  ])


AC_DEFUN([IX_PATH_LIBPNG],[
  IX_PATH_ANY_LIBRARY([libpng],[png],[-lz],[png.h],,[png_read_info],
    [present="yes"
    AC_DEFINE(SDLUCID_HAS_LIBPNG,,[libpng found?])],
    [present="no"])
  AM_CONDITIONAL(HAVE_LIBPNG,test "x$present" = "xyes")
  ])
AC_DEFUN([IX_PATH_FREETYPE],[
  IX_PATH_ANY_LIBRARY([freetype],[ttf],,[freetype/freetype.h],,[TT_Init_FreeType],
    [present="yes"
    AC_DEFINE(SDLUCID_HAS_FREETYPE,,[freetype found?])],
    [present="no"])
  AM_CONDITIONAL(HAVE_FREETYPE,test "x$present" = "xyes")
  ])
AC_DEFUN([IX_PATH_LIBMIKMOD],[
  IX_PATH_ANY_LIBRARY([libmikmod],[mikmod],,[mikmod.h],,[MikMod_Init],
    [present="yes"
    AC_DEFINE(SDLUCID_HAS_LIBMIKMOD,,[mikmod found?])],
    [present="no"])
  AM_CONDITIONAL(HAVE_LIBMIKMOD,test "x$present" = "xyes")
  ])
