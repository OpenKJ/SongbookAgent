#!/bin/bash

set -e

# do not build mac for PR
if [ "${TRAVIS_PULL_REQUEST}" != "false" ]; then
  exit 0
fi

BundlePath=$PWD/SongbookAgent.app

$HOME/Qt/5.10.0/clang_64/bin/qmake

make -j3

mv SongbookAgent.app "Songbook Agent.app"

$HOME/Qt/5.10.0/clang_64/bin/macdeployqt "Songbook Agent.app"
echo "Removing unneeded and non-appstore compliant plugins"
rm -f "Songbook Agent.app/Contents/PlugIns/sqldrivers/libqsqlmysql.dylib"
rm -f "Songbook Agent.app/Contents/PlugIns/sqldrivers/libqsqlodbc.dylib"
rm -f "Songbook Agent.app/Contents/PlugIns/sqldrivers/libqsqlpsql.dylib"

echo "Signing code"
codesign -s "Application: Isaac Lightburn (47W8CPBS5A)" -vvvv --deep --timestamp=none "Songbook Agent.app"
echo "Creating installer"
cp travis/dmgbkg.png ~/
appdmg travis/openkj-songbook-agent-dmg.json ${INSTALLERFN} 
echo "Signing installer"
codesign -s "Application: Isaac Lightburn (47W8CPBS5A)" -vvvv --timestamp=none ${INSTALLERFN}

mkdir -p deploy/macos/${BRANCH}
mv ${INSTALLERFN} deploy/macos/${BRANCH}/
