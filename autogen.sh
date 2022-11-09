#!/bin/bash
libtoolize --force
#autoheader
aclocal
touch stamp-h NEWS README AUTHORS ChangeLog
automake --add-missing --gnu
autoconf
