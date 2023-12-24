#!/bin/sh

#set -x

VERSIONDEFSHEADER="../src/versiondefs.h"
VERSIONINFOBATFILE="versioninfo.bat"

WD=`dirname "$0"`

cd "$WD"

read_variable ()
{
	grep "$1" "$VERSIONINFOBATFILE" | sed -e 's/.*=\(.*\)/\1/' | tr -d '
'
}

VER_MAJ=`read_variable VER_MAJ`
VER_MIN=`read_variable VER_MIN`
VER_PAT=`read_variable VER_PAT`
COMPANY=`read_variable COMPANY_NAME`
DOMAIN_NAME=`read_variable DOMAIN_NAME`

VERSION="$VER_MAJ.$VER_MIN.$VER_PAT"

if ! grep -q "$VERSION" "$VERSIONDEFSHEADER"
then
echo "#ifndef VERSIONDEFS_H
#define VERSIONDEFS_H

#define VER_MAJ		$VER_MAJ
#define VER_MIN		$VER_MIN
#define VER_PAT		$VER_PAT
#define FAMILIARAPP	\"Familiar\"
#define FAMILIARORG	\"$COMPANY\"
#define FAMILIARDOMAIN	\"$DOMAIN_NAME\"

#endif // VERSIONDEFS_H" > $VERSIONDEFSHEADER
fi

cd -

