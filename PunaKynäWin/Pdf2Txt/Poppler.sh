#!/bin/sh
appname=`pdftotext.exe`

dirname=`dirname $0`
tmp="${dirname#?}"

if [ "${dirname%$tmp}" != "/" ]; then
dirname=$PWD/$dirname
fi

export DYLD_LIBRARY_PATH=$dirname/popplerlibs
$dirname/$appname "$@"
