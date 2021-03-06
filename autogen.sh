#!/bin/sh

set -e

run() {
  echo "    $@"
  echo "    -----------------------------------------------"
  "$@"
  }

run aclocal -I $HOME/pool/share/aclocal
run autoheader
libtoolize --force
# yes no | run gettextize -f
run autoconf
run automake -a
rm -f config.cache

echo
echo "ready to run configure."
