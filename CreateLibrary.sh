#!/bin/sh
LIBRARY_NAME="wrfarduinolib"
WRF_SDK=SDK
ARDUINO_SDK=SDK.ARDUINO
EXPORT_DIR=$LIBRARY_NAME
cp -R $ARDUINO_SDK $LIBRARY_NAME
cp -a $WRF_SDK/. $LIBRARY_NAME/src