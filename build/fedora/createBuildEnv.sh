#!/bin/sh
#
# The following packages are required before executing this script!
#
# yum -y update
# yum -y install rpmdevtools tinyxml-devel zlib-devel libusb-devel garmintools-devel xulrunner-devel openssl-devel
# yum -y groupinstall "Development Tools"
#


FEDORAVERSION=$1

GITREPOSITORYDIR=`pwd`

if [ -z $FEDORAVERSION"" ]; then
  echo "Please specify for which fedora version you want to build" 
  echo "Example: createBuildEnv.sh 14"
  exit 1
fi

if [ -d "$GITREPOSITORYDIR/$FEDORAVERSION/SPECS/" ]; then
  echo "Using build environment from directory $GITREPOSITORYDIR/$FEDORAVERSION/SPECS/"
else
  echo "Build environment settings not found in directory $GITREPOSITORYDIR/$FEDORAVERSION/SPECS/"
  exit 1
fi

# Determine last tag
GITTAG=`git tag -l | tail -1`
VERSION=`echo $GITTAG|cut -c2-`
ARCHITECTURE=`uname -i`
echo "Creating build environment for version $VERSION - architecture $ARCHITECTURE"

# Checkout last version
#git checkout $GITTAG

cd
HOMEDIR=`pwd`

# Create Build Environment
rpmdev-setuptree

if [ -d "$HOMEDIR/rpmbuild/SOURCES/" ]; then
  echo "Build environment created $HOMEDIR/rpmbuild/SOURCES/"
else
  echo "Failed to create build environment. Missing directory $HOMEDIR/rpmbuild/SOURCES/"
  exit 1
fi

echo "Copying Sources to build dir $HOMEDIR/rpmbuild/SOURCES"

if [ -d "$HOMEDIR/rpmbuild/SOURCES/GarminPlugin-$VERSION" ]; then
  echo "Removing previously created directory $HOMEDIR/rpmbuild/SOURCES/GarminPlugin-$VERSION"
  rm -r "$HOMEDIR/rpmbuild/SOURCES/GarminPlugin-$VERSION"
fi

cp -r "$GITREPOSITORYDIR/../../src" "$HOMEDIR/rpmbuild/SOURCES/GarminPlugin-$VERSION"

if [ -f "$HOMEDIR/rpmbuild/SOURCES/GarminPlugin-$VERSION.tar.gz" ]; then
  echo "Removing previously created tar $HOMEDIR/rpmbuild/SOURCES/GarminPlugin-$VERSION.tar.gz"
  rm "$HOMEDIR/rpmbuild/SOURCES/GarminPlugin-$VERSION.tar.gz"
fi
cd "$HOMEDIR/rpmbuild/SOURCES/"
tar cvzf "GarminPlugin-$VERSION.tar.gz" "GarminPlugin-$VERSION"

# removing no longer needed directory with sources
rm -r "$HOMEDIR/rpmbuild/SOURCES/GarminPlugin-$VERSION"

# Building rpms
# http://www.deaconsworld.org.uk/2009/02/05/building-rpms-as-a-normal-user/


SPECFILE="$HOMEDIR/rpmbuild/SPECS/GarminPlugin.spec"
cp "$GITREPOSITORYDIR/$FEDORAVERSION/SPECS/GarminPlugin.spec" "$SPECFILE"

# Replace Version and architecture in file
sed -i "s/§§§VERSION§§§/$VERSION/g" "$SPECFILE"
sed -i "s/§§§ARCHITECTURE§§§/$ARCHITECTURE/g" "$SPECFILE"


INFILE="$GITREPOSITORYDIR/../../HISTORY"
while read curline; do
    if [ "x$curline" != "x" ]; then
        FIRSTCHAR=`echo $curline|cut -b1`
        if [ "$FIRSTCHAR" != "-" ]; then
            DATE=`echo $curline | cut -b-10`
            DATE=`date "+%a %b %d %Y" -d $DATE`
            echo "* $DATE Andreas Diesner <garminplugin@andreas-diesner.de>" >>$SPECFILE 
        else
            echo $curline >>$SPECFILE 
        fi
    fi
done < $INFILE

rm "$HOMEDIR/rpmbuild/RPMS/GarminPlugin-$VERSION*"
cd $HOMEDIR/rpmbuild/SPECS

# Build env is ready

echo "********************************************************"
echo "Build environment setup complete"
echo "Now enter the following commands to start your build:"
echo ""
echo "cd '$HOMEDIR/rpmbuild/SPECS'"
echo "rpmbuild --clean GarminPlugin.spec"
echo "rpmbuild -bb GarminPlugin.spec"
