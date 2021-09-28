#!/bin/bash

# do not build mac for PR
if [ "${TRAVIS_PULL_REQUEST}" != "false" ]; then
  exit 0
fi

echo "PWD: $PWD"
echo "BRANCH_BUCKET=${BRANCH_BUCKET}"

echo "Setting up signing environment"
security create-keychain -p $keychainPass build.keychain
security default-keychain -s build.keychain
security unlock-keychain -p $keychainPass build.keychain
security set-keychain-settings -t 3600 -u build.keychain

security import applekey.p12 -k build.keychain -P $p12Pass -A 
security set-key-partition-list -S apple-tool:,apple: -s -k $keychainPass build.keychain

#echo "Installing osxrelocator"
#pip2 install osxrelocator

echo "Installing appdmg"
npm install -g appdmg

if [ -d "Qt" ]; then
  echo "Cached copy of Qt already exists, skipping install"
else
  #install gstreamer#install Qt
  echo "Downloading Qt"
  wget -c --no-check-certificate -nv -Oqt.tar.bz2 https://cloud.hm.hozed.net/index.php/s/3lyFyolHbBdMx8o/download
  echo "Extracting Qt"
  bunzip2 qt.tar.bz2
  tar -xf qt.tar
  echo "Moving Qt to proper location"
  mv Qt $HOME/Qt
fi

