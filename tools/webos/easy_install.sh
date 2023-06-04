#!/bin/sh

ARES_DEVICE=$(ares-device -d | xargs) cmake --build .. --target ihsplay-launch
