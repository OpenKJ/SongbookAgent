#!/bin/bash

export PATH=$PATH:$QT_MACOS/bin
if [ "${TRAVIS_BRANCH}" == "release" ]; then
  export BRANCH="release"
else
  export BRANCH="unstable"
fi
LC_REPO_SLUG=$(echo "$TRAVIS_REPO_SLUG" | tr '[:upper:]' '[:lower:]')
LC_REPO_SLUG="${LC_REPO_SLUG}-${TRAVIS_BRANCH}"
export BRANCH_BUCKET=$(echo $LC_REPO_SLUG | sed -e 's/\//-/g') 
export INSTALLERFN="OpenKJ-Songbook-Agent-${SBAVER}-${BRANCH}-osx-installer.dmg"

chmod 755 ./travis/install.sh
chmod 755 ./travis/build.sh

openssl aes-256-cbc -K $encrypted_87fb155abb03_key -iv $encrypted_87fb155abb03_iv -in travis/MacSigningCert.p12.enc -out applekey.p12 -d
