#!/bin/bash

if [ -z "$1" ]
  then
    echo "First argument must be nonempty password"
    exit 1
fi

keytool -genkey -v -keystore cvlab-android-prod.keystore \
  -alias cvlab_release \
  -keyalg RSA \
  -keysize 2048 \
  -validity 10000 \
  -storepass $1 \
  -dname "CN=Mikhail Burakov, OU=Dev, O=Burakov, L=St Petersburg, ST=Russia, C=RU"

keytool -genkey -v -keystore cvlab-android-prod.keystore \
  -alias cvlab_stage \
  -keyalg RSA \
  -keysize 2048 \
  -validity 10000 \
  -storepass $1 \
  -dname "CN=Mikhail Burakov, OU=Dev, O=Burakov, L=St Petersburg, ST=Russia, C=RU"

keytool -genkey -v -keystore cvlab-android-dev.keystore \
  -alias cvlab_debug \
  -keyalg RSA \
  -keysize 2048 \
  -validity 10000 \
  -storepass cvlab_debug \
  -dname "CN=Mikhail Burakov, OU=Dev, O=Burakov, L=St Petersburg, ST=Russia, C=RU"


