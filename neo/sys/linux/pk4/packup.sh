#!/bin/sh
rm $1
flip -u $2
mkdir -p `dirname $1`
cd /var/local/Doom/base
cat $2 | while read i ; do find . -ipath "./$i" ; done | cut -b 3- | tee /home/timo/cased.log | zip $1 -@

