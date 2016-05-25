#!/usr/bin/env bash

packageName=hithere
sourceVersion=1.0
packageRevision=1
maintainerName="Ryan Niehaus"
maintainerEmail="ospb@nieha.us"
packageShortDescription="greet user"
packageLongDescription="$packageName greets the user, or the world."

sourceDir=./"$packageName"-"$sourceVersion"

pushd "$sourceDir" > /dev/null
rm -rf debian
mkdir -p debian/source

#builds changelog
dch --create -v "$sourceVersion"-"$packageRevision" --package "$packageName"

echo 9 > debian/compat

echo "Source: $packageName" > debian/control
echo "Maintainer: $maintainerName <$maintainerEmail>" >> debian/control
echo "Section: misc" >> debian/control
# required, important, standard, optional, extra
echo "Priority: optional" >> debian/control
echo "Standards-Version: 3.9.2" >> debian/control
echo "Build-Depends: debhelper (>= 9)" >> debian/control
echo >> debian/control
echo "Package: $packageName" >> debian/control
# i386, amd64, etc... any means can be built for any... all means the same binary package works on all
echo "Architecture: any" >> debian/control
echo "Depends: ${shlibs:Depends}, ${misc:Depends}" >> debian/control
echo "Description: $packageShortDescription" >> debian/control
echo "$packageLongDescription" >> debian/control

echo > debian/copyright

echo "#!/usr/bin/make -f" > debian/rules
echo "%:" >> debian/rules
echo -e "\tdh \$@" >> debian/rules
echo >> debian/rules
echo "override_dh_auto_install:" >> debian/rules
echo -e "\t\$(MAKE) DESTDIR=\$\$(pwd)/debian/$packageName prefix=/usr install" >> debian/rules

echo "3.0 (quilt)" > debian/source/format

echo "usr/bin" > debian/"$packageName".dirs
echo "usr/share/man/man1" >> debian/"$packageName".dirs

debuild -us -uc

popd > /dev/null

