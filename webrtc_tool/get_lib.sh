#!/bin/sh
mkdir ../lib
src=`find -name *.a`
for var in $src
do
    cp $var ../lib
done
