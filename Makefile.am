# -----------------------------------------------------------------------------
# SDLucid root makefile
# -----------------------------------------------------------------------------
# (c) Andreas Kloeckner 2001
# -----------------------------------------------------------------------------

SUBDIRS=src examples

EXTRA_DIST=sdlucid.m4 sdlucid.lsm

m4datadir=$(datadir)/aclocal
m4data_DATA=sdlucid.m4

bin_SCRIPTS=sdlucid-config

include_HEADERS=sdlucid_config.hh

update-web-site: 
	rsync -e ssh --verbose --progress web/* inducer@shell.sourceforge.net:/home/groups/s/sd/sdlucid/htdocs
