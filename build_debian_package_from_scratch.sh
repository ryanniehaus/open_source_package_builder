#!/usr/bin/env bash

#GLOBALpackageName
#GLOBALsourceVersion
#GLOBALpackageRevision
#GLOBALmaintainerName
#GLOBALmaintainerEmail
#GLOBALpackageShortDescription
#GLOBALpackageLongDescription

export EDITOR=$(pwd)/temp_editor.sh
echo "#!/bin/bash" > temp_editor.sh
echo "echo Running $@" >> temp_editor.sh
echo "echo \" \" >> \"$2\"" >> temp_editor.sh
echo "exit 0" >> temp_editor.sh
echo "" >> temp_editor.sh
chmod ug+rx ./temp_editor.sh

if [ -d ]
then
	rm -rf debian
fi
mkdir -p debian/source

export DEBFULLNAME="$GLOBALmaintainerName"
export DEBEMAIL="$GLOBALmaintainerEmail"

#builds changelog
dch --create \
	--version "$GLOBALsourceVersion"-"$GLOBALpackageRevision" \
	--package "$GLOBALpackageName" \
	--distribution "Unstable" \
	--force-distribution \
	"Autobuilt version of $GLOBALpackageName v$GLOBALsourceVersion from upstream"
tempRetval="$?"

if [ ! "$tempRetval" == "0" ]
then
  echo "dch FAILED"
  echo "dch DUMP START ---"
  cat -n $(which dch)
  echo "dch DUMP END ---"
fi

ls -la
ls -la debian

echo 9 > debian/compat

echo "Source: $GLOBALpackageName" > debian/control
echo "Maintainer: $GLOBALmaintainerName <$GLOBALmaintainerEmail>" >> debian/control
echo "Section: misc" >> debian/control
# required, important, standard, optional, extra
echo "Priority: optional" >> debian/control
echo "Standards-Version: 3.9.2" >> debian/control
echo "Build-Depends: debhelper (>= 9)" >> debian/control
echo >> debian/control
echo "Package: $GLOBALpackageName" >> debian/control
# i386, amd64, etc... any means can be built for any... all means the same binary package works on all
echo "Architecture: any" >> debian/control
echo "Depends: ${shlibs:Depends}, ${misc:Depends}" >> debian/control
echo "Description: $GLOBALpackageShortDescription" >> debian/control
echo "$GLOBALpackageLongDescription" >> debian/control

echo > debian/copyright

echo "#!/usr/bin/make -f" > debian/rules
echo "%:" >> debian/rules
echo -e "\tdh \$@" >> debian/rules
echo >> debian/rules
echo "override_dh_auto_install:" >> debian/rules
echo -e "\t\$(MAKE) DESTDIR=\$\$(pwd)/debian/$GLOBALpackageName prefix=/usr install" >> debian/rules

echo "3.0 (quilt)" > debian/source/format

echo "usr/bin" > debian/"$GLOBALpackageName".dirs
echo "usr/share/man/man1" >> debian/"$GLOBALpackageName".dirs

ls -la
ls -la debian

debuild -us -uc

