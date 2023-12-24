#!/bin/bash

#set -x

REBUILD=0
CLEAN=0
VERSION=0
VER_MAJ=0
VER_MIN=91
VER_PAT=10000

usage ()
{
	echo
	echo "Usage:"
	echo "		$0 [ -r ] -v VERSION"
	echo "		$0 -c"
	echo "		$0 -h"
	echo
	echo "		-r : Cleans build environment and starts the build again."
	echo "		-v VERSION : The version to be used for build."
	echo "		-c : Cleans build environment."
	echo "		-h : Displays this help."
	echo
	echo "		VERSION: is version string. e.g. $VER_MAJ.$VER_MIN.$VER_PAT"
	echo
}

if [ $# -eq 0 ]
then
	usage 1>&2
	exit 1
fi

while [ $# -gt 0 ]
do
	case "$1" in
		-h)	usage
			exit 0
			;;
		-r)	REBUILD=1
			;;
		-c)	CLEAN=1
			;;
		-v)	VER_STR="$2"
			if [ -z "$VER_STR" -o ! -z "`echo $VER_STR | sed -e 's/[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*//'`" ]
			then
				echo "$VER_STR is not a proper version string." 1>&2
				usage 1>&2
				exit 1
			fi
			VERSION=1
			VER_MAJ=`echo $VER_STR | cut -d'.' -f1`
			VER_MIN=`echo $VER_STR | cut -d'.' -f2`
			VER_PAT=`echo $VER_STR | cut -d'.' -f3`
			VER_BLD=`echo $VER_STR | cut -d'.' -f4` 

			shift
			;;
		*)	echo "$1 is not a supported parameter. Aborting." 1>&2
			usage 1>&2
			exit 1
			;;
	esac

	shift
done

BUILDSCRIPTSDIR="buildscripts"
VERSIONDEFS_H_PATH="src/versiondefs.h"
APPINFO_RC_PATH="rc/appinfo.rc"
WINE=`which wine`
COMPANY="69Solutions"
DOMAIN_NAME="example.com"


if [ $CLEAN -eq 1 -o $REBUILD -eq 1 ]
then
	echo "Cleaning UNIX binaries ..."
        qmake Familiar.pro
        make -j${nproc} clean
	rm -f Makefile

	if [ -x "$WINE" ]
	then
		echo "Cleaning Windows binaries ..."
		$WINE cmd /c $BUILDSCRIPTSDIR/build-with-wine-clean.bat
                rm -f Makefile Makefile.Release Makefile.Debug object_script.Familiar.Debug object_script.Familiar.Release
	else	
		echo "wine not found! Skipping windows cleaning."
	fi
fi

if [ $CLEAN -eq 0 ]
then
	if [ $VERSION -eq 0 ]
	then
		echo "Build requires a version to be set." 1>&2
		usage 1>&2
		exit 1
	fi


	echo "Setting up build environment"

	cat >"$VERSIONDEFS_H_PATH" << EOF
#ifndef VERSIONDEFS_H
#define VERSIONDEFS_H

#define VER_MAJ		$VER_MAJ
#define VER_MIN		$VER_MIN
#define VER_PAT		$VER_PAT
#define VER_BLD		$VER_BLD
#define FAMILIARAPP	"Familiar"
#define FAMILIARORG	"$COMPANY"
#define FAMILIARDOMAIN	"$DOMAIN_NAME"

#endif // VERSIONDEFS_H" > $VERSIONDEFSHEADER
EOF

	cat >"$APPINFO_RC_PATH" << EOF
# if defined(UNDER_CE)
#  include "winbase.h"
# else
#  include "winver.h"
# endif
VS_VERSION_INFO VERSIONINFO
	FILEVERSION $VER_MAJ,$VER_MIN,$VER_PAT,$VER_BLD
	PRODUCTVERSION $VER_MAJ,$VER_MIN,$VER_PAT,$VER_BLD
	FILEFLAGSMASK 0x3fL
#ifdef _DEBUG
	FILEFLAGS VS_FF_DEBUG
#else
	FILEFLAGS 0x0L
#endif
	FILEOS VOS__WINDOWS32
	FILETYPE VFT_DLL
	FILESUBTYPE 0x0L
	BEGIN
		BLOCK "StringFileInfo"
		BEGIN
			BLOCK "040904B0"
			BEGIN
				VALUE "CompanyName", "$COMPANY\\0"
                                VALUE "FileDescription", "Familiar Executable\\0"
				VALUE "FileVersion", "$VER_MAJ.$VER_MIN.$VER_PAT.$VER_BLD\\0"
				VALUE "LegalCopyright", "Copyright (c) Neeraj Jakhar 2009-`date +%y`.\\0"
                                VALUE "OriginalFilename", "Familiar.exe\\0"
                                VALUE "ProductName", "Familiar\\0"
			END
		END
	END
/* End of Version info */
IDI_ICON1               ICON    DISCARDABLE     "familiar.ico"
EOF

	echo "Building UNIX binaries ..."
	date
        qmake Familiar.pro
	make -f Makefile buildinfo
	make -j6 -f Makefile
	
	PACKAGEDIR="package"
        PACKAGE="familiar-$VER_MAJ.$VER_MIN.$VER_PAT.$VER_BLD"
	TEMPSRCDIR="$PACKAGEDIR/$PACKAGE"
	mkdir -p "$TEMPSRCDIR"

        cp "Familiar.pro" AUTHORS COPYING INSTALL README ChangeLog "$TEMPSRCDIR"
	mkdir -p "$TEMPSRCDIR/src"
	cp -a src/* "$TEMPSRCDIR/src"
	mkdir -p "$TEMPSRCDIR/rc"
	cp -a rc/* "$TEMPSRCDIR/rc"
	mkdir -p "$TEMPSRCDIR/desktop"
	cp -a desktop/* "$TEMPSRCDIR/desktop"
	mkdir -p "$TEMPSRCDIR/examples"
	cp -a examples/* "$TEMPSRCDIR/examples"
	
	cd "$PACKAGEDIR"
	rm -f "$PACKAGE.tar.bz2"
	tar cjf "$PACKAGE.tar.bz2" "$PACKAGE"
	rm -rf "$PACKAGE"
	MD5SUM=`md5sum -b "$PACKAGE.tar.bz2" | cut -d' ' -f1`
	cd -

        ARCHLINUX_PACKAGE_DIRECTORY="installer/unix/archlinux/familiar"
	mkdir -p "$ARCHLINUX_PACKAGE_DIRECTORY"
        rm -f "$ARCHLINUX_PACKAGE_DIRECTORY"/familiar-*.tar.*
	cp "$PACKAGEDIR/$PACKAGE.tar.bz2" "$ARCHLINUX_PACKAGE_DIRECTORY"
	cd "$ARCHLINUX_PACKAGE_DIRECTORY"
	sed -i -e "s/^pkgver=.*/pkgver=$VER_MAJ.$VER_MIN.$VER_PAT.$VER_BLD/" -e "s/^md5sums=('[^']*')/md5sums=('$MD5SUM')/" "PKGBUILD"
	rm -rf pkg src
	makepkg -f
	rm -rf pkg src
        sudo pacman -U -v --noconfirm "familiar-$VER_MAJ.$VER_MIN.$VER_PAT.$VER_BLD-1-i686.pkg.tar.xz"
	cd -

	date
	echo
	echo

	if [ -x "$WINE" ]
	then
		echo "Building Windows binaries ..."
		date
		$WINE cmd /c $BUILDSCRIPTSDIR/build-with-wine.bat
		date
		echo
		echo
	else
		echo "wine not found! Skipping windows build."
	fi
fi

