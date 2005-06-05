# -----------------------------------------------------------------------------
# SDLucid example makefile
# -----------------------------------------------------------------------------
# (c) Andreas Kloeckner 2001
# -----------------------------------------------------------------------------

# Targets ---------------------------------------------------------------------
CXXFLAGS=@CXXFLAGS@ -I$(top_srcdir)/src @SDL_CFLAGS@ @IXLIB_CFLAGS@ @LIBPNG_CFLAGS@ \
  @FREETYPE_CFLAGS@ @SMPEG_CFLAGS@ @LIBMIKMOD_CFLAGS@

# Tools -----------------------------------------------------------------------
noinst_PROGRAMS=example

example_SOURCES=example.cc refpoint.hh refpoint.cc
example_LDADD=../src/libsdlucid.la

EXTRA_DIST=logo.png logo.xcf vgademo.mod tahoma.fnt tahoma.png README \
  night-in-samarkand.png