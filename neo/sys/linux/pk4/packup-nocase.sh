#!/bin/sh
rm $1
flip -u $2
mkdir -p `dirname $1`
cd /var/local/Doom/base
cat $2 | zip $1 -@

