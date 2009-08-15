aclocal ${ACLOCAL_FLAGS}
libtoolize --force --copy
autoheader
automake --add-missing --copy --foreign
autoconf
./configure $@
