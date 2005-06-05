# -----------------------------------------------------------------------------
# SDLucid source makefile
# -----------------------------------------------------------------------------
# (c) Andreas Kloeckner 2001
# -----------------------------------------------------------------------------

# -----------------------------------------------------------------------------
base_headers=sdlucid_base.hh \
  sdlucid_instance.hh \
  sdlucid_video.hh \
  sdlucid_audio.hh \
  sdlucid_threads.hh \
  sdlucid_iostream_rwops.hh \
  sdlucid_png.hh 
base_modules=base.cc \
  instance.cc \
  video.cc \
  audio.cc \
  threads.cc \
  iostream_rwops.cc \
  png.cc 

if HAVE_LIBMIKMOD
mikmod_headers=sdlucid_mikmod_stream.hh
mikmod_modules=mikmod_stream.cc
mikmod_libraries=-lmikmod
else
mikmod_headers=
mikmod_modules=
mikmod_libraries=
endif
mikmod_extra_files=sdlucid_mikmod_stream.hh mikmod_stream.cc


if HAVE_SMPEG
smpeg_headers=sdlucid_smpeg_stream.hh
smpeg_modules=smpeg_stream.cc
smpeg_libraries=-lsmpeg
else
smpeg_headers=
smpeg_modules=
smpeg_libraries=
endif
smpeg_extra_files=sdlucid_smpeg_stream.hh smpeg_stream.cc


# Targets ---------------------------------------------------------------------
CXXFLAGS=@CXXFLAGS@ @SDL_CFLAGS@ @IXLIB_CFLAGS@ @LIBPNG_CFLAGS@ \
  @FREETYPE_CFLAGS@ @SMPEG_CFLAGS@ @LIBMIKMOD_CFLAGS@

lib_LTLIBRARIES=libsdlucid.la

include_HEADERS=$(base_headers) $(mikmod_headers) $(smpeg_headers)

libsdlucid_la_SOURCES=$(base_headers) $(base_modules) \
  $(mikmod_headers) $(mikmod_modules)\
  $(smpeg_headers) $(smpeg_modules)
libsdlucid_la_LDFLAGS=-version-info @BINARY_IFACE_VERSION@:@BINARY_REVISION@:@BINARY_AGE@ \
  @LIBPNG_LDFLAGS@ @LIBMIKMOD_LDFLAGS@ @SMPEG_LDFLAGS@
libsdlucid_la_LIBADD=$(mikmod_libraries) $(smpeg_libraries) @LIBPNG_LIBS@ \
  @LIBMIKMOD_LIBS@ @SMPEG_LIBS@ @IXLIB_LIBS@ 

# Tools -----------------------------------------------------------------------
freetype_extra_files=ripttf.cc

if HAVE_FREETYPE
freetype_programs=sdlucid_ripttf
else
endif

bin_PROGRAMS=$(freetype_programs)

sdlucid_ripttf_SOURCES=ripttf.cc
sdlucid_ripttf_LDFLAGS=@LIBPNG_LDFLAGS@ @FREETYPE_LDFLAGS@
sdlucid_ripttf_LDADD=libsdlucid.la \
  @SDL_LIBS@ @IXLIB_LIBS@ @LIBPNG_LIBS@ @FREETYPE_LIBS@

EXTRA_DIST=$(freetype_extra_files) $(smpeg_extra_files) $(mikmod_extra_files)
