#!/bin/sh
#
# This file downloads additional files needed to build for lucid
#
# $1 = Directory where this file is located
# $2 = Target directory
#
HOMEDIR=$1
TARGETDIR=$2

cd $TARGETDIR
echo "Downloading tinyxml from Sourceforege"
wget "http://downloads.sourceforge.net/project/tinyxml/tinyxml/2.6.1/tinyxml_2_6_1.tar.gz?r=http%3A%2F%2Fsourceforge.net%2Fprojects%2Ftinyxml%2Ffiles%2Ftinyxml%2F2.6.1%2F&ts=1301068190&use_mirror=heanet" -O tinyxml_2_6_1.tar.gz
if [ -f "$TARGETDIR/tinyxml_2_6_1.tar.gz" ]; then
  tar xzf ./tinyxml_2_6_1.tar.gz
  rm ./tinyxml_2_6_1.tar.gz

  sed -i "s/DEBUG_CXXFLAGS   := /DEBUG_CXXFLAGS   := -fPIC /" tinyxml/Makefile
  sed -i "s/RELEASE_CXXFLAGS := /RELEASE_CXXFLAGS := -fPIC /" tinyxml/Makefile
  sed -i "s/TINYXML_USE_STL := NO/TINYXML_USE_STL := YES/" tinyxml/Makefile
else
  echo "************ ERROR ERROR ************"
  echo "Unable to download tinyxml_2_6_1.tar.gz from Sourceforge using wget! Check your internet connection!"
  echo "************ ERROR ERROR ************"
  exit 1
fi

