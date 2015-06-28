#!/bin/sh
#
# The following packages are required before executing this script!
#
# apt-get update
# apt-get install build-essential subversion git-core xulrunner-dev zlib1g-dev libusb-dev
#


DEBIANVERSION=$1
VERSION=$2
GITREPOSITORYDIR=`pwd`

if [ -z $DEBIANVERSION"" ]; then
  echo "Please specify for which debian version you want to build"
  echo "Example: createBuildEnv.sh maverick 0.3.0"
  exit 1
fi

if [ -f "$GITREPOSITORYDIR/$DEBIANVERSION/debian/control" ]; then
  echo "Using build environment from directory $GITREPOSITORYDIR/$DEBIANVERSION/"
else
  echo "Build environment settings not found in directory $GITREPOSITORYDIR/$DEBIANVERSION/debian/"
  exit 1
fi

if [ ! -n "$VERSION" ]; then
  # Determine last tag
  GITTAG=`git tag -l | tail -1`
  VERSION=`echo $GITTAG|cut -c2-`
  echo "Please specify which plugin version you want to build"
  echo "Example: createBuildEnv.sh maverick $VERSION"
  exit 1
fi

ARCHITECTURE=`uname -i`
echo "Creating build environment for version $VERSION - architecture $ARCHITECTURE"

# Checkout last version
#git checkout $GITTAG

cd
HOMEDIR=`pwd`

# Create Build Environment
cd "$GITREPOSITORYDIR/../.."
mkdir "$GITREPOSITORYDIR/../../pbuilder"
cd "$GITREPOSITORYDIR/../../pbuilder"
mkdir "garminplugin-$VERSION"
cp -r "$GITREPOSITORYDIR/$DEBIANVERSION/debian" "./garminplugin-$VERSION/debian"

echo "Copying Sources..."

cp -r "$GITREPOSITORYDIR/../../src" "./garminplugin-$VERSION/GarminPlugin"

cd "garminplugin-$VERSION"
TARGETDIR=`pwd`

if [ -f "$GITREPOSITORYDIR/$DEBIANVERSION/buildEnv.sh" ]; then
  echo "Executing $DEBIANVERSION specific settings"
  "$GITREPOSITORYDIR/$DEBIANVERSION/buildEnv.sh" "$GITREPOSITORYDIR/$DEBIANVERSION" "$TARGETDIR"
fi

cd "$GITREPOSITORYDIR/../../pbuilder"

# now write changelog file
INFILE="$GITREPOSITORYDIR/../../HISTORY"
CHANGELOGFILE="$GITREPOSITORYDIR/../../pbuilder/garminplugin-$VERSION/debian/changelog"
echo "Creating changelog file $CHANGELOGFILE"
DOOUTPUT="0"
while read curline; do
    if [ "a$curline" != "a" ]; then
        FIRSTCHAR=`echo $curline|cut -b1`
        if [ "a$FIRSTCHAR" != "a-" ]; then
            VERSIONNUMBERENTRY=`echo $curline | cut -b22-`
            if [ "a$VERSIONNUMBERENTRY" != "a$VERSION" ]; then
                if [ "a$DOOUTPUT" != "a0" ]; then
                    DOOUTPUT="0"
                    echo "">>$CHANGELOGFILE
                    echo " -- Andreas Diesner <garminplugin@andreas-diesner.de>  $DATE"
                    echo " -- Andreas Diesner <garminplugin@andreas-diesner.de>  $DATE">>$CHANGELOGFILE
                fi
            else
                DATE=`echo $curline | cut -b-10`
                DATE=`date -R -d $DATE`
                DOOUTPUT="1"
                echo "garminplugin ($VERSION-1~$DEBIANVERSION) $DEBIANVERSION; urgency=low">$CHANGELOGFILE
                echo "garminplugin ($VERSION-1~$DEBIANVERSION) $DEBIANVERSION; urgency=low"
                echo "">>$CHANGELOGFILE
            fi
        else
            if [ "a$DOOUTPUT" != "a0" ]; then
                curline=`echo $curline | cut -b2-`
                echo "  *$curline" >>$CHANGELOGFILE
                echo "  *$curline"
            fi
        fi
    fi
done < $INFILE


# Build env is ready
cd "$GITREPOSITORYDIR/../../pbuilder"
LOCALDIR=`pwd`

echo "********************************************************"
echo "Build environment setup complete"
echo "Now enter the following commands to start your build:"
echo ""
echo "cd '$LOCALDIR'"
echo "dpkg-source -b garminplugin-$VERSION"
echo "sudo DIST=$DEBIANVERSION ARCH=i386 pbuilder build garminplugin_$VERSION-1~$DEBIANVERSION.dsc"
