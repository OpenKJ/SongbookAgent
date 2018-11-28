#!/bin/bash

set -e

# do not build mac for PR
if [ "${TRAVIS_PULL_REQUEST}" != "false" ]; then
  exit 0
fi

BundlePath=$PWD/SongbookClient.app

$HOME/Qt/5.10.0/clang_64/bin/qmake

make -j3

$HOME/Qt/5.10.0/clang_64/bin/macdeployqt ${BundlePath}
echo "Removing unneeded and non-appstore compliant plugins"
rm -f ${BundlePath}/Contents/PlugIns/sqldrivers/libqsqlmysql.dylib
rm -f ${BundlePath}/Contents/PlugIns/sqldrivers/libqsqlodbc.dylib
rm -f ${BundlePath}/Contents/PlugIns/sqldrivers/libqsqlpsql.dylib

echo "Signing code"
codesign -s "Application: Isaac Lightburn (47W8CPBS5A)" -vvvv --deep --timestamp=none ${BundlePath}
echo "Creating installer"
cp travis/dmgbkg.png ~/
appdmg travis/openkj-songbook-agent-dmg.json ${INSTALLERFN} 
echo "Signing installer"
codesign -s "Application: Isaac Lightburn (47W8CPBS5A)" -vvvv --timestamp=none ${INSTALLERFN}

mkdir -p deploy/macos/${BRANCH}
mv ${INSTALLERFN} deploy/macos/${BRANCH}/
