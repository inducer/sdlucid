dnl AM_PATH_SDLUCID([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND [, MODULES]]]])
dnl Test for sdlucid, and define SDLUCID_CFLAGS, SDLUCID_LIBS
dnl
AC_DEFUN(AM_PATH_SDLUCID,
[dnl 
dnl Get the cflags and libraries from the sdlucid-config script
dnl
AC_ARG_WITH(sdlucid-prefix,[  --with-sdlucid-prefix=PFX   Prefix where SDLUCID is installed (optional)],
            sdlucid_config_prefix="$withval", sdlucid_config_prefix="")
AC_ARG_WITH(sdlucid-exec-prefix,[  --with-sdlucid-exec-prefix=PFX Exec prefix where SDLUCID is installed (optional)],
            sdlucid_config_exec_prefix="$withval", sdlucid_config_exec_prefix="")
AC_ARG_ENABLE(sdlucidtest, [  --disable-sdlucidtest       Do not try to compile and run a test SDLUCID program],
		    , enable_sdlucidtest=yes)

  if test x$sdlucid_config_exec_prefix != x ; then
     sdlucid_config_args="$sdlucid_config_args --exec-prefix=$sdlucid_config_exec_prefix"
     if test x${SDLUCID_CONFIG+set} != xset ; then
        SDLUCID_CONFIG=$sdlucid_config_exec_prefix/bin/sdlucid-config
     fi
  fi
  if test x$sdlucid_config_prefix != x ; then
     sdlucid_config_args="$sdlucid_config_args --prefix=$sdlucid_config_prefix"
     if test x${SDLUCID_CONFIG+set} != xset ; then
        SDLUCID_CONFIG=$sdlucid_config_prefix/bin/sdlucid-config
     fi
  fi

  AC_PATH_PROG(SDLUCID_CONFIG, sdlucid-config, no)
  min_sdlucid_version=ifelse([$1], , 0.90.0, $1)
  AC_MSG_CHECKING(for sdlucid version >= $min_sdlucid_version)
  no_sdlucid=""
  if test "$SDLUCID_CONFIG" = "no" ; then
    no_sdlucid=yes
  else
    SDLUCID_CFLAGS=`$SDLUCID_CONFIG $sdlucid_config_args --cflags`
    SDLUCID_LIBS=`$SDLUCID_CONFIG $sdlucid_config_args --libs`
    sdlucid_config_major_version=`$SDLUCID_CONFIG $sdlucid_config_args --version | \
	   sed -e 's,[[^0-9.]],,g' -e 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    sdlucid_config_minor_version=`$SDLUCID_CONFIG $sdlucid_config_args --version | \
	   sed -e 's,[[^0-9.]],,g' -e 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    sdlucid_config_micro_version=`$SDLUCID_CONFIG $sdlucid_config_args --version | \
	   sed -e 's,[[^0-9.]],,g' -e 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    if test "x$enable_sdlucidtest" = "xyes" ; then
      ac_save_CXXFLAGS="$CXXFLAGS"
      ac_save_LIBS="$LIBS"
      CXXFLAGS="$CXXFLAGS $SDLUCID_CFLAGS"
      LIBS="$SDLUCID_LIBS $LIBS"
dnl
dnl Now check if the installed SDLUCID is sufficiently new. (Also sanity
dnl checks the results of sdlucid-config to some extent
dnl
      rm -f conf.sdlucidtest
      AC_LANG_SAVE
      AC_LANG_CPLUSPLUS
      AC_TRY_RUN([
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sdlucid_base.hh>
#include <sdlucid_config.hh>
using namespace ixion;

int 
main ()
{
  int major, minor, micro;
  char *tmp_version;

  system ("touch conf.sdlucidtest");

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = strdup("$min_sdlucid_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string\n", "$min_sdlucid_version");
     return 1;
   }

  if ((sdlucidGetMajorVersion() != $sdlucid_config_major_version) ||
      (sdlucidGetMinorVersion() != $sdlucid_config_minor_version) ||
      (sdlucidGetMicroVersion() != $sdlucid_config_micro_version))
    {
      printf("\n*** 'sdlucid-config --version' returned %d.%d.%d, but version %d.%d.%d\n", 
             $sdlucid_config_major_version, $sdlucid_config_minor_version, $sdlucid_config_micro_version,
             sdlucidGetMajorVersion(),sdlucidGetMinorVersion(),sdlucidGetMicroVersion());
      printf ("*** was found! If the script was correct, then it is best\n");
      printf ("*** to remove the old version of the software. You may also be able to fix the error\n");
      printf("*** by modifying your LD_LIBRARY_PATH enviroment variable, or by editing\n");
      printf("*** /etc/ld.so.conf. Make sure you have run ldconfig if that is\n");
      printf("*** required on your system.\n");
      printf("*** If the script was wrong, set the environment variable SDLUCID_CONFIG\n");
      printf("*** to point to the correct copy of sdlucid-config, and remove the file config.cache\n");
      printf("*** before re-running configure\n");
    } 
  else if ((SDLUCID_MAJOR_VERSION != sdlucidGetMajorVersion()) ||
	   (SDLUCID_MINOR_VERSION != sdlucidGetMinorVersion()) ||
           (SDLUCID_MICRO_VERSION != sdlucidGetMicroVersion()))
    {
      printf("*** header files (version %d.%d.%d) do not match\n",
	     SDLUCID_MAJOR_VERSION,SDLUCID_MINOR_VERSION,SDLUCID_MICRO_VERSION);
      printf("*** library (version %d.%d.%d)\n",
	     sdlucidGetMajorVersion(),sdlucidGetMinorVersion(),sdlucidGetMicroVersion());
    }
  else
    {
      if ((sdlucidGetMajorVersion() > major) ||
         ((sdlucidGetMajorVersion() == major) && (sdlucidGetMinorVersion() > minor)) ||
         ((sdlucidGetMajorVersion() == major) && (sdlucidGetMinorVersion() == minor) && (sdlucidGetMicroVersion() >= micro)))
      {
        return 0;
       }
     else
      {
        printf("\n*** An old version of the software (%d.%d.%d) was found.\n",
               sdlucidGetMajorVersion(), sdlucidGetMinorVersion(),sdlucidGetMicroVersion());
        printf("*** You need version %d.%d.%d or greater of sdlucid. The latest version of\n",
	       major, minor, micro);
        printf("*** the software is always available from http://sdlucid.sourceforge.net/.\n");
        printf("***\n");
        printf("*** If you have already installed a sufficiently new version, this error\n");
        printf("*** probably means that the wrong copy of the sdlucid-config shell script is\n");
        printf("*** being found. The easiest way to fix this is to remove the old version\n");
        printf("*** of the software, but you can also set the SDLUCID_CONFIG environment to point to the\n");
        printf("*** correct copy of sdlucid-config. (In this case, you will have to\n");
        printf("*** modify your LD_LIBRARY_PATH enviroment variable, or edit /etc/ld.so.conf\n");
        printf("*** so that the correct libraries are found at run-time))\n");
      }
    }
  return 1;
}
],, no_sdlucid=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CXXFLAGS="$ac_save_CXXFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_sdlucid" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$SDLUCID_CONFIG" = "no" ; then
       echo "*** The sdlucid-config script could not be found."
       echo "*** If the package was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the SDLUCID_CONFIG environment variable to the"
       echo "*** full path to sdlucid-config."
     else
       if test -f conf.sdlucidtest ; then
        :
       else
          echo "*** Could not run test program, checking why..."
          CXXFLAGS="$CXXFLAGS $SDLUCID_CFLAGS"
          LIBS="$LIBS $SDLUCID_LIBS"
          AC_TRY_LINK([
#include <cstdio>
#include <sdlucid_base.hh>
using namespace ixion;
],      [ return ((sdlucidGetMajorVersion()) || (sdlucidGetMinorVersion()) || (sdlucidGetMicroVersion())); ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding the package or finding the wrong"
          echo "*** version of it. If it is not finding it, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"
          echo "***" ],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means the package was incorrectly"
          echo "*** installed or that you have moved sdlucid since it was installed. In the latter"
          echo "*** case, you may want to edit the sdlucid-config script: $SDLUCID_CONFIG" ])
          CXXFLAGS="$ac_save_CXXFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     SDLUCID_CFLAGS=""
     SDLUCID_LIBS=""
     ifelse([$3], , :, [$3])
  fi

  AC_LANG_RESTORE

  if test "x$no_sdlucid" = "xyes" ; then 
	exit 1
  fi

  AC_SUBST(SDLUCID_CFLAGS)
  AC_SUBST(SDLUCID_LIBS)
  rm -f conf.sdlucidtest
])
