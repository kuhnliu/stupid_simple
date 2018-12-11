#!/bin/sh
src=`find -name *.a`
for var in $src
do
    ar x $var
done
ar cru libwebrtc_all.a *.o
ranlib libwebrtc_all.a
